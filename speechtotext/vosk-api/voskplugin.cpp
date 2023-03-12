#include "voskplugin.h"

#include <QDir>
#include <QJsonDocument>
#include <QLocale>

#include "vosk_api.h" // Include the Vosk API header file

// Declare global variables for the Vosk model and recognizer
VoskModel *_model = nullptr;
VoskRecognizer *_globalRecognizer = nullptr;

VoskPlugin::VoskPlugin(QObject *parent)
    : SpeechToTextPlugin(parent)
{
    open(QIODevice::ReadWrite);

    vosk_set_log_level(-1);
};

qint64 VoskPlugin::writeData(const char *data, qint64 size)
{
    if (vosk_recognizer_accept_waveform(_globalRecognizer, data, (int) size))
        parseText(vosk_recognizer_result(_globalRecognizer));
    else
        parsePartial(vosk_recognizer_partial_result(_globalRecognizer));

    return size;
}

void VoskPlugin::parseText(const char *json)
{
    auto obj = QJsonDocument::fromJson(json);
    QString text = obj[QStringLiteral("text")].toString();

    if (text.isEmpty())
        return;
    else if (m_isAsking) {
        Q_EMIT answerReady(text);
        return;
    }

    text.append(u' ');

    if (!text.contains(m_wakeWord))
        return;

    text = text.mid(text.indexOf(m_wakeWord) + m_wakeWord.size());
    text = text.trimmed();

    Q_EMIT textUpdated(text);
    qDebug() << "[debug] Text:" << text;
    Q_EMIT doneListening();
}

void VoskPlugin::parsePartial(const char *json)
{
    auto obj = QJsonDocument::fromJson(json);
    QString text = obj[QStringLiteral("partial")].toString();
    if (text.isEmpty())
        return;
    text.append(u' ');

    if (text.contains(m_wakeWord)) {
        Q_EMIT wakeWordDetected();
        text = text.mid(text.indexOf(m_wakeWord) + m_wakeWord.size());
    } else if (!m_isAsking)
        return;

    Q_EMIT textUpdated(text);
}

void VoskPlugin::setup(const QString &modelDir, bool *success)
{
    qDebug() << "[debug] Setting up model and recognizer";

    const QStringList uiLangs = QLocale::system().uiLanguages();

    QDir dir(modelDir);
    if (dir.isEmpty(QDir::Dirs)) {
        setState(ModelsMissing);
        *success = false;
        return;
    }

    for (const auto &lang : uiLangs) {
        QString formattedLang = lang.toLower().replace(u'_', u'-');
        if (!QDir(modelDir + formattedLang).exists())
            continue;

        _model = vosk_model_new(QString(modelDir + formattedLang).toUtf8());
        if (_model) {
            qDebug() << "[debug] Loaded model, language:" << lang;
            _globalRecognizer = vosk_recognizer_new(_model, 16000.0);
        }

        if (!_model || !_globalRecognizer)
            continue;

        m_language = lang;

        qDebug() << "[debug] SpeechToText loaded successful";

        Q_EMIT loaded();
        setState(Running);
        *success = true;
        return;
    }

    setState(NoModelFound);
    qDebug() << "[debug] No model found!";
    *success = false;
}

bool VoskPlugin::canRecognizeWord(const QString &word)
{
    if (!_model)
        return false;

    if (word.isEmpty())
        return true;

    return vosk_model_find_word(_model, word.toLower().toUtf8()) != -1;
}

void VoskPlugin::setState(State s)
{
    m_state = s;

    switch (m_state) {
    case Running:
        m_errorString.clear();
        break;
    case NoModelFound:
        m_errorString = tr("No Vosk model was found for the system languages");
        break;
    case ModelsMissing:
        m_errorString = tr("The directory where the Vosk models are stored is empty");
        break;
    case ErrorWhileLoading:
        m_errorString = tr(
            "An unknown error occurred while loading the Vosk model and/or recognizer");
        break;
    case NotStarted:
        m_errorString = tr("The recognizer has not yet been set up");
        break;
    case Paused:
        m_errorString = tr("The audio device is closed, and is not processing any audio data");
        break;
    default:
        break;
    }

    qDebug() << "[plugin] Vosk state changed:" << s << ":" << m_errorString;

    Q_EMIT stateChanged();
}

VoskPlugin::~VoskPlugin()
{
    vosk_recognizer_free(_globalRecognizer);
    vosk_model_free(_model);
}
