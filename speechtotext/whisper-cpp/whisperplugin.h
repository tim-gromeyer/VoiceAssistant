#pragma once

#include "../speechtotextplugin.h"
#include <whisper.h>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QQueue>
#include <QThread>
#include <QTimer>

class WhisperPlugin : public SpeechToTextPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID SpeechToText_iid)
    Q_INTERFACES(SpeechToTextPlugin)

public:
    explicit WhisperPlugin(QObject *parent = nullptr)
        : SpeechToTextPlugin(parent)
        , m_ctx(nullptr)
        , m_state(NotStarted)
        , m_sampleRate(16000)
        , m_asking(false)
    {
        m_audioBuffer.reserve(30 * m_sampleRate); // 30-second buffer
    }

    ~WhisperPlugin() override
    {
        if (m_ctx)
            whisper_free(m_ctx);
    }

    QString pluginName() const override { return "WhisperSTT"; }

    void setup(const QString &modelDir, bool *success) override
    {
        QMutexLocker locker(&m_mutex);
        if (m_ctx)
            whisper_free(m_ctx);

        *success = false;
        QDir dir(modelDir);

        if (!dir.exists()) {
            m_state = ModelsMissing;
            m_errorString = "Model directory does not exist";
            emit stateChanged();
            return;
        }

        // Look for ggml model files
        QStringList models = dir.entryList({"*.bin"}, QDir::Files);
        if (models.isEmpty()) {
            m_state = ModelsMissing;
            m_errorString = "No model files found";
            emit stateChanged();
            return;
        }

        QString modelPath = dir.filePath(models.first());
        whisper_context_params cparams = whisper_context_default_params();
        cparams.use_gpu = true;
        m_ctx = whisper_init_from_file_with_params(modelPath.toUtf8().constData(), cparams);

        if (!m_ctx) {
            *success = false;
            setState(ErrorWhileLoading);
            return;
        }

        Q_EMIT loaded();
        *success = true;
        setState(Running);
        QTimer::singleShot(0, this, &WhisperPlugin::processAudio);
    }

    qint64 writeData(const char *data, qint64 size) override
    {
        QMutexLocker locker(&m_mutex);
        if (m_state != Running)
            return size;

        const int16_t *samples = reinterpret_cast<const int16_t *>(data);
        const qint64 numSamples = size / sizeof(int16_t);

        m_audioBuffer.insert(m_audioBuffer.end(), samples, samples + numSamples);

        // Handle buffer overflow
        if (m_audioBuffer.size() > 30 * SAMPLE_RATE) {
            const size_t remove = m_audioBuffer.size() - 30 * SAMPLE_RATE;
            m_audioBuffer.erase(m_audioBuffer.begin(), m_audioBuffer.begin() + remove);
            m_lastProcessed = std::max<size_t>(m_lastProcessed, remove) - remove;
        }

        return size;
    }

    // Other required implementations
    State state() override { return m_state; }
    QString errorString() override { return m_errorString; }
    QString language() override { return "en"; }
    QString wakeWord() override { return m_wakeWord; }
    void setWakeWord(const QString &word) override
    {
        m_wakeWord = word.toLower();
        emit wakeWordChanged();
    }
    bool isAsking() override { return m_asking; }
    void setAsking(bool asking) override
    {
        m_asking = asking;
        emit askingChanged();
    }
    bool hasLookupSupport() override { return false; }
    bool canRecognizeWord(const QString &) override { return true; }
    void clear() override
    {
        QMutexLocker locker(&m_mutex);
        m_audioBuffer.clear();
    }
    int sampleRate() override { return m_sampleRate; }

private slots:
    void processAudio()
    {
        processAudioChunk();
        QTimer::singleShot(m_isListening ? PROCESS_INTERVAL_ACTIVE : PROCESS_INTERVAL_IDLE,
                           this,
                           m_isListening ? &WhisperPlugin::processRealTime
                                         : &WhisperPlugin::processAudio);
    }

    void handleVoiceActivity() { processAudioChunk(true); }

private:
    void setState(State newState)
    {
        m_state = newState;
        emit stateChanged();
    }

    QMutex m_mutex;
    whisper_context *m_ctx;
    State m_state;
    QString m_errorString;
    QString m_wakeWord;
    std::vector<int16_t> m_audioBuffer;
    const int m_sampleRate;
    bool m_asking;

    static std::string transcribe(whisper_context *ctx,
                                  const std::vector<float> &pcmf32,
                                  const std::string &grammar_rule,
                                  float &logprob_min,
                                  float &logprob_sum,
                                  int &n_tokens,
                                  int64_t &t_ms)
    {
        const auto t_start = std::chrono::high_resolution_clock::now();

        logprob_min = 0.0f;
        logprob_sum = 0.0f;
        n_tokens = 0;
        t_ms = 0;

        //whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_BEAM_SEARCH);

        wparams.print_progress = false;
        wparams.print_realtime = false;
        wparams.no_context = true;
        wparams.single_segment = true;

        wparams.temperature = 0.4f;
        wparams.temperature_inc = 1.0f;
        wparams.greedy.best_of = 5;

        wparams.beam_search.beam_size = 5;

        if (whisper_full(ctx, wparams, pcmf32.data(), pcmf32.size()) != 0) {
            return "";
        }

        std::string result;

        const int n_segments = whisper_full_n_segments(ctx);
        for (int i = 0; i < n_segments; ++i) {
            const char *text = whisper_full_get_segment_text(ctx, i);

            result += text;

            const int n = whisper_full_n_tokens(ctx, i);
            for (int j = 0; j < n; ++j) {
                const auto token = whisper_full_get_token_data(ctx, i, j);

                if (token.plog > 0.0f)
                    exit(0);
                logprob_min = std::min(logprob_min, token.plog);
                logprob_sum += token.plog;
                ++n_tokens;
            }
        }

        const auto t_end = std::chrono::high_resolution_clock::now();
        t_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();

        return result;
    }

    void high_pass_filter(std::vector<float> &data, float cutoff, float sample_rate)
    {
        const float rc = 1.0f / (2.0f * M_PI * cutoff);
        const float dt = 1.0f / sample_rate;
        const float alpha = dt / (rc + dt);

        float y = data[0];

        for (size_t i = 1; i < data.size(); i++) {
            y = alpha * (y + data[i] - data[i - 1]);
            data[i] = y;
        }
    }

    bool vad_simple(std::vector<float> &pcmf32,
                    int sample_rate,
                    int last_ms,
                    float vad_thold,
                    float freq_thold,
                    bool verbose)
    {
        const int n_samples = pcmf32.size();
        const int n_samples_last = (sample_rate * last_ms) / 1000;

        if (n_samples_last >= n_samples) {
            // not enough samples - assume no speech
            return false;
        }

        if (freq_thold > 0.0f) {
            high_pass_filter(pcmf32, freq_thold, sample_rate);
        }

        float energy_all = 0.0f;
        float energy_last = 0.0f;

        for (int i = 0; i < n_samples; i++) {
            energy_all += fabsf(pcmf32[i]);

            if (i >= n_samples - n_samples_last) {
                energy_last += fabsf(pcmf32[i]);
            }
        }

        energy_all /= n_samples;
        energy_last /= n_samples_last;

        if (verbose) {
            fprintf(stderr,
                    "%s: energy_all: %f, energy_last: %f, vad_thold: %f, freq_thold: %f\n",
                    __func__,
                    energy_all,
                    energy_last,
                    vad_thold,
                    freq_thold);
        }

        if (energy_last > vad_thold * energy_all) {
            return false;
        }

        return true;
    }

private:
    static constexpr int CHUNK_SIZE_MS = 1000; // Process 1-second chunks
    static constexpr int SAMPLE_RATE = 16000;  // 16kHz sample rate
    static constexpr int CHUNK_SAMPLES = SAMPLE_RATE * CHUNK_SIZE_MS / 1000;
    static constexpr int PROCESS_INTERVAL_ACTIVE = 500; // 50ms for active mode
    static constexpr int PROCESS_INTERVAL_IDLE = 5000;  // 500ms for idle mode

    bool m_isListening = false;
    size_t m_lastProcessed = 0;
    std::vector<float> m_contextBuffer; // For maintaining audio context

    void processAudioChunk(bool force = false)
    {
        QMutexLocker locker(&m_mutex);
        if (m_state != Running)
            return;

        const size_t available = m_audioBuffer.size() - m_lastProcessed;
        const size_t required = CHUNK_SAMPLES;

        if (available >= required || force) {
            // Get audio chunk with context
            std::vector<float> pcmf32;
            const size_t context_size = m_contextBuffer.size();
            pcmf32.reserve(context_size + CHUNK_SAMPLES);

            // Add previous context
            pcmf32.insert(pcmf32.end(), m_contextBuffer.begin(), m_contextBuffer.end());

            // Add new samples
            auto start = m_audioBuffer.begin() + m_lastProcessed;
            auto end = start + std::min<size_t>(available, required);
            for (auto it = start; it != end; ++it) {
                pcmf32.push_back(*it / 32768.0f);
            }

            // Keep last 30% as new context
            const size_t keep_samples = static_cast<size_t>(pcmf32.size() * 0.3);
            m_contextBuffer.clear();
            if (pcmf32.size() > keep_samples) {
                m_contextBuffer.insert(m_contextBuffer.end(),
                                       pcmf32.end() - keep_samples,
                                       pcmf32.end());
            }

            // Update processed position
            m_lastProcessed += (end - start);

            // Perform transcription
            float logprob, logprob_sum;
            int n_tokens;
            int64_t t_ms;
            QString text = QString::fromStdString(
                               transcribe(m_ctx, pcmf32, "", logprob, logprob_sum, n_tokens, t_ms))
                               .trimmed();

            if (!text.isEmpty()) {
                if (m_isListening) {
                    emit textUpdated(text);
                } else if (text.contains(m_wakeWord, Qt::CaseInsensitive)) {
                    m_isListening = true;
                    emit wakeWordDetected();
                    QTimer::singleShot(0, this, [this]() {
                        // Switch to fast processing timer
                        QTimer::singleShot(PROCESS_INTERVAL_ACTIVE,
                                           this,
                                           &WhisperPlugin::processRealTime);
                    });
                } else {
                    emit falsePositiveWakeWord();
                }
            }

            // Check for silence to stop listening
            if (m_isListening && !vad_simple(pcmf32, SAMPLE_RATE, 1000, 0.6f, 100.0f, false)) {
                m_isListening = false;
                m_contextBuffer.clear();
                emit doneListening();
            }
        }
    }

    void processRealTime()
    {
        processAudioChunk();
        if (m_isListening) {
            QTimer::singleShot(PROCESS_INTERVAL_ACTIVE, this, &WhisperPlugin::processRealTime);
        }
    }
};

Q_PLUGIN_METADATA(WhisperPlugin)
