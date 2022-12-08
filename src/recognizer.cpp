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

Listener::Listener(QObject *parent)
    : QIODevice(parent) {
    open(QIODevice::ReadWrite);
};

qint64 Listener::writeData(const char *data, qint64 size) {
    if (vosk_recognizer_accept_waveform(globalRecognizer, data, size))
        parseText(vosk_recognizer_final_result(globalRecognizer));
    else
        parsePartial(vosk_recognizer_partial_result(globalRecognizer));

    return size;
}

void Listener::parseText(const char *json) {
    QJsonDocument obj = QJsonDocument::fromJson(json);
    const QString text = obj[L1("text")].toString();

    waitWakeWort = true;

    if (text.isEmpty()) return;

    qDebug() << "[debug] Text:" << text;
    Q_EMIT textUpdated(text);

    Q_EMIT doneListening();
}

void Listener::parsePartial(const char *json) {
    QJsonDocument obj = QJsonDocument::fromJson(json);
    const QString text = obj[L1("partial")].toString();
    if (text.isEmpty()) return;

    if (text.contains(L1("alexa"))) {
        vosk_recognizer_reset(globalRecognizer);
        Q_EMIT wakeWord();
    }

    Q_EMIT textUpdated(text);
}

Recognizer::Recognizer(QObject *parent)
    : QObject{parent}
{
    device = new Listener(this);
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
    qDebug() << "[debug] Prepare microphone";

    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    audio = new QAudioSource(QMediaDevices::defaultAudioInput(), format, this);
    connect(audio, &QAudioSource::stateChanged, this, [](QAudio::State state){
        qDebug() << "[debug] Microphone state:" << state;
    });
    audio->setBufferSize(8000);
    audio->start(device);

    qDebug() << "[debug] Microphone set up";
}

Recognizer::~Recognizer()
{
    // if (globalRecognizer) vosk_recognizer_free(globalRecognizer);
    if (model) vosk_model_free(model);

    device->deleteLater();

    delete device;
}

#include "recognizer.moc"
