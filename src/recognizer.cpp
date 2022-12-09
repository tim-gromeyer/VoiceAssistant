#include "recognizer.h"
#include "global.h"

#include <QAudioSource>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QLocale>
#include <QMediaDevices>
#include <QMessageBox>
#include <QThread>


VoskRecognizer *globalRecognizer = nullptr;
QLatin1String _wakeWord = L1("alexa ");
qsizetype _wakeWordSize = 6;

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
    : QObject{parent}, device(new Listener(this))
{
}

void Recognizer::setUpModel()
{
    qDebug() << "[debug] Setting up model and recognizer";

    const QStringList uiLangs = QLocale::system().uiLanguages();

#ifdef QT_DEBUG
    QString appDir = STR(APP_DIR);
#else
    QString appDir = QCoreApplication::applicationDirPath();
#endif
    appDir.append(L1("/models/"));

    QDir dir(appDir);
    if (dir.isEmpty(QDir::Dirs)) {
        emit stateChanged(ModelsMissing);
        return;
    }

    for (const auto &lang : uiLangs) {
        if (!dir::exists(appDir + lang)) continue;

        model = vosk_model_new(QString(appDir + lang).toUtf8());
        if (model) {
            qDebug() << "[debug] Loaded model, language:" << lang;

            globalRecognizer = vosk_recognizer_new(model, 16000.0);
        }

        if (!model || !globalRecognizer) {
            emit stateChanged(ErrorWhileLoading);
            return;
        }

        qDebug() << "[debug] Recognizer loaded successful";

        language = lang;
        Q_EMIT stateChanged(Ok);
        break;
    }

    if (language.isEmpty()) {
        emit stateChanged(NoModelFound);
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
    format.setSampleFormat(QAudioFormat::Int16);

    audio.reset(new QAudioSource(format, this));
    connect(audio.get(), &QAudioSource::stateChanged, this, [](QAudio::State state){
        qDebug() << "[debug] Microphone state:" << state;
    });
    audio->setBufferSize(8000);
    audio->start(device.get());

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
