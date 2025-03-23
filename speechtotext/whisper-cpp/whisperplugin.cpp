#include "whisperplugin.h"
#include <QDebug>
#include <QDir>

WhisperPlugin::WhisperPlugin(QObject *parent)
    : SpeechToTextPlugin(parent)
    , m_state(NotStarted)
    , m_asking(false)
{
    open(QIODevice::WriteOnly);
}

WhisperPlugin::~WhisperPlugin()
{
    if (m_worker) {
        m_worker->stop();
        m_worker->wait();
    }
}

void WhisperPlugin::setup(const QString &modelDir, bool *success)
{
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

    m_modelPath = dir.filePath(models.first());
    m_worker = std::make_unique<WhisperWorker>(this);

    if (m_worker.get() == nullptr) {
        m_state = ErrorWhileLoading;
        m_errorString = "Failed to load whisper model";
        emit stateChanged();
        return;
    }

    m_state = Running;
    *success = true;
    emit loaded();
    emit stateChanged();
}

qint64 WhisperPlugin::writeData(const char *data, qint64 size)
{
    if (m_state != Running || !m_worker) {
        return size;
    }

    m_buffer.append(data, size);

    // Process complete chunks
    while (m_buffer.size() >= BUFFER_SIZE) {
        QByteArray chunk = m_buffer.left(BUFFER_SIZE);
        m_buffer.remove(0, BUFFER_SIZE);
        m_worker->processAudio(chunk);
    }

    return size;
}

void WhisperPlugin::setWakeWord(const QString &word)
{
    if (m_wakeWord != word) {
        m_wakeWord = word;
        emit wakeWordChanged();
    }
}

void WhisperPlugin::setAsking(bool asking)
{
    if (m_asking != asking) {
        m_asking = asking;
        emit askingChanged();
    }
}

bool WhisperPlugin::canRecognizeWord(const QString &word)
{
    // Whisper should be able to recognize most words in its training vocabulary
    return !word.isEmpty();
}

void WhisperPlugin::clear()
{
    m_buffer.clear();
    if (m_worker) {
        m_worker->clear();
    }
}

// WhisperWorker implementation
WhisperPlugin::WhisperWorker::WhisperWorker(WhisperPlugin *plugin)
    : m_plugin(plugin)
    , m_running(true)
    , m_ctx(whisper_init_from_file(plugin->m_modelPath.toUtf8().constData()), &whisper_free)
{
    // Initialize whisper parameters
    m_params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    m_params.print_realtime = true;
    m_params.print_progress = false;
    m_params.print_timestamps = false;
    m_params.translate = false;
    m_params.language = "auto";
    m_params.n_threads = QThread::idealThreadCount();

    start();
}

WhisperPlugin::WhisperWorker::~WhisperWorker()
{
    stop();
    wait();
}

void WhisperPlugin::WhisperWorker::processAudio(const QByteArray &audio)
{
    QMutexLocker locker(&m_mutex);
    m_audioQueue.enqueue(audio);
}

void WhisperPlugin::WhisperWorker::clear()
{
    m_audioQueue.clear();
}

void WhisperPlugin::WhisperWorker::stop()
{
    QMutexLocker locker(&m_mutex);
    m_running = false;
}

void WhisperPlugin::WhisperWorker::run()
{
    while (m_running) {
        QByteArray audio;
        {
            QMutexLocker locker(&m_mutex);
            if (!m_audioQueue.isEmpty()) {
                audio = m_audioQueue.dequeue();
            }
        }

        if (!audio.isEmpty() && m_ctx) {
            // Convert audio to float array
            const int16_t *samples = reinterpret_cast<const int16_t *>(audio.constData());
            int n_samples = audio.size() / sizeof(int16_t);
            std::vector<float> pcm(n_samples);
            for (int i = 0; i < n_samples; ++i) {
                pcm[i] = static_cast<float>(samples[i]) / 32768.0f;
            }

            // Process audio with whisper
            if (whisper_full(m_ctx.get(), m_params, pcm.data(), pcm.size()) == 0) {
                // Get the transcribed text
                int n_segments = whisper_full_n_segments(m_ctx.get());
                QString text;
                for (int i = 0; i < n_segments; ++i) {
                    text += QString::fromUtf8(whisper_full_get_segment_text(m_ctx.get(), i));
                }

                if (!text.isEmpty()) {
                    // Check for wake word if not in asking mode
                    if (!m_plugin->m_asking && !m_plugin->m_wakeWord.isEmpty()
                        && text.toLower().contains(m_plugin->m_wakeWord.toLower())) {
                        emit m_plugin->wakeWordDetected();
                    }

                    emit m_plugin->textUpdated(text.trimmed());
                }
            }
        }

        // Sleep briefly to prevent busy-waiting
        msleep(10);
    }
}
