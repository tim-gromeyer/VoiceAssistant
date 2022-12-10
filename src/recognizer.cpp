#include "recognizer.h"
#include "global.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QLocale>
#include <QMessageBox>
#include <QThread>

#include "vosk_api.h"


VoskModel *model = nullptr;
VoskRecognizer *globalRecognizer = nullptr;
QLatin1String _wakeWord = L1("alexa ");
int _wakeWordSize = 6;
QString _modelDir;

Listener::Listener(QObject *parent)
    : QIODevice(parent) {
    open(QIODevice::ReadWrite);
};

qint64 Listener::writeData(const char *data, qint64 size) {
    if (vosk_recognizer_accept_waveform(globalRecognizer, data, (int)size))
        parseText(vosk_recognizer_final_result(globalRecognizer));
    else
        parsePartial(vosk_recognizer_partial_result(globalRecognizer));

    return size;
}

void Listener::parseText(const char *json) {
    QJsonDocument obj = QJsonDocument::fromJson(json);
    QString text = obj[L1("text")].toString();

    waitWakeWort = true;

    Q_EMIT textUpdated(text);

    if (text.isEmpty()) return;

    if (text.contains(_wakeWord))
        text = text.mid(text.indexOf(_wakeWord) + _wakeWord.size());

    qDebug() << "[debug] Text:" << text;

    Q_EMIT doneListening();
}

void Listener::parsePartial(const char *json) {
    QJsonDocument obj = QJsonDocument::fromJson(json);
    QString text = obj[L1("partial")].toString();
    if (text.isEmpty()) return;

    if (text.contains(_wakeWord)) {
        Q_EMIT wakeWord();
        text = text.mid(text.indexOf(_wakeWord) + _wakeWord.size());
    }

    Q_EMIT textUpdated(text);
}

Recognizer::Recognizer(QObject *parent)
    : QObject{parent}, m_device(new Listener(this))
{
    // Disable kaldi debug messages
    vosk_set_log_level(-1);
}

void Recognizer::setUpModel()
{
    qDebug() << "[debug] Setting up model and recognizer";

    const QStringList uiLangs = QLocale::system().uiLanguages();

    QDir dir(modelDir());
    if (dir.isEmpty(QDir::Dirs)) {
        setState(ModelsMissing);
        return;
    }

    for (const auto &lang : uiLangs) {
        if (!dir::exists(modelDir() + lang)) continue;

        model = vosk_model_new(QString(modelDir() + lang).toUtf8());
        if (model) {
            qDebug() << "[debug] Loaded model, language:" << lang;

            globalRecognizer = vosk_recognizer_new(model, 16000.0);
        }

        if (!model || !globalRecognizer) {
            setState(ErrorWhileLoading);
            return;
        }

        qDebug() << "[debug] Recognizer loaded successful";

        language = lang;
        setState(Ok);
        break;
    }

    if (language.isEmpty()) {
        setState(NoModelFound);
        qDebug() << "[debug] No model found!";
    }
}

void Recognizer::setUpMic()
{
    if (!globalRecognizer) return;

    qDebug() << "[debug] Prepare microphone";

    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    format.setSampleFormat(QAudioFormat::Int16);
#else
    format.setSampleSize(16);
#endif

    audio.reset(new AUDIOINPUT(format, this));
    connect(audio.get(), &AUDIOINPUT::stateChanged, this, [](QAudio::State state){
        qDebug() << "[debug] Microphone state:" << state;
    });
    audio->setBufferSize(8000);
    audio->start(m_device.get());

    qDebug() << "[debug] Microphone set up";
}

bool Recognizer::hasWord(QString word) {
    if (!model) return false;

    if (word.isEmpty()) return true;
    word = word.toLower();

    return vosk_model_find_word(model, word.toUtf8()) != -1;
}

void Recognizer::setWakeWord(const QString &word) {
    _wakeWord = QLatin1String(word.toLatin1() + ' ');
    _wakeWordSize = _wakeWord.size();
}

QString Recognizer::wakeWord() {
    return _wakeWord.left(_wakeWordSize -1);
}

void Recognizer::setModelDir(const QString &dir) {
    _modelDir = dir;
}

QString Recognizer::modelDir() {
    if (_modelDir.isEmpty()) {
#ifdef QT_DEBUG
        _modelDir = STR(APP_DIR);
#else
        _modelDir = QCoreApplication::applicationDirPath();
#endif
        _modelDir.append(L1("/models/"));
    }

    return _modelDir;
}

Recognizer::~Recognizer()
{
    // if (globalRecognizer) vosk_recognizer_free(globalRecognizer);
    if (model) vosk_model_free(model);
}
