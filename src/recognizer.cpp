#include "recognizer.h" // Include the header file for the SpeechToText class
#include "global.h"
#include "pluginloader.h"
#include "speechtotext/speechtotextplugin.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QLocale>
#include <QMessageBox>
#include <QPluginLoader>
#include <QtConcurrentRun>

#if NEED_MICROPHONE_PERMISSION
#include <QPermission>
#endif

SpeechToText::SpeechToText(const QString &pluginName, QObject *parent)
    : QObject{parent}
{
    // Create loader for speech-to-text plugins
    auto *loader = new PluginLoader<SpeechToTextPlugin>(this);
    loader->loadPlugins(dir::speechToTextPluginDir());
    m_plugins = loader->plugins();

    if (m_plugins.isEmpty()) {
        qCritical() << "No speech to text plugin found!";
        setState(NoPluginFound);
        return;
    }

    if (m_plugin == nullptr)
        m_plugin = m_plugins.last();

    m_plugin->open(QIODevice::ReadWrite);

    connect(m_plugin, &SpeechToTextPlugin::answerReady, this, &SpeechToText::onAnswerReady);
    connect(m_plugin, &SpeechToTextPlugin::stateChanged, this, &SpeechToText::pluginStateChanged);
    connect(m_plugin, &SpeechToTextPlugin::falsePositiveWakeWord, this, [this] {
        qDebug() << "SpeechToText: False positive wake word";
        Q_EMIT m_plugin->textUpdated({});
        onAnswerReady({});
    });
}

void SpeechToText::onAnswerReady(const QString &answer)
{
    if (m_plugin)
        m_plugin->setAsking(false);
    Q_EMIT answerReady(answer);
}

void SpeechToText::ask()
{
    if (m_plugin)
        m_plugin->setAsking(true);
}

SpeechToText::operator bool() const
{
    return audio && m_plugin;
}

void SpeechToText::pause()
{
    if (m_plugin && m_plugin->isAsking())
        Q_EMIT answerReady(QLatin1String());

    m_muted = true;

    if (!audio)
        return;

    reset();
    // audio->stop(); // This somehow crashes
    QMetaObject::invokeMethod(
        audio, [this] { audio->stop(); }, Qt::QueuedConnection);

    if (m_plugin)
        m_plugin->setAsking(false);

    setState(Paused);
}

void SpeechToText::resume()
{
    if (!audio || m_state == PermissionMissing)
        return;

    m_muted = false;

    switch (audio->state()) {
    case QAudio::ActiveState:
        return;
    case QAudio::SuspendedState:
        audio->resume();
        break;
    case QAudio::StoppedState:
        audio->start(m_plugin);
        break;
    default:
        break;
    }

    setState(Running);
}

void SpeechToText::reset()
{
    if (!m_plugin || !audio)
        return;

    audio->reset();

    m_plugin->reset();
    m_plugin->clear();

    if (audio->state() == QAudio::StoppedState)
        resume();
}

bool SpeechToText::setUpModel()
{
    if (!m_plugin) {
        setState(State::PluginError);
        return false;
    }

    bool success = false;
    m_plugin->setup(dir::modelDir(), &success);
    if (success)
        Q_EMIT languageChanged();

    return success;
}

void SpeechToText::setUpMic()
{
    if (m_state != NotStarted || !m_plugin)
        return;

    qDebug() << "[debug] Prepare microphone";

    QAudioFormat format;
    format.setSampleRate(m_plugin->sampleRate());
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);

    auto inputDevices = QMediaDevices::audioInputs();
    auto input = QMediaDevices::defaultAudioInput();

    if (inputDevices.isEmpty()) {
        setState(NoMicrophone);
        return;
    } else if (!input.isFormatSupported(format)) {
        setState(IncompatibleFormat);
        return;
    }

    audio = new QAudioSource(format, this);
    connect(audio, &QAudioSource::stateChanged, this, [](QAudio::State state) {
        qDebug() << "[debug] Microphone state:" << state;
    });
    audio->setBufferSize(8000);

    if (m_muted)
        setState(Paused);
    else {
        audio->start(m_plugin);
        setState(Running);
    }

    qDebug() << "[debug] Microphone set up";
}

void SpeechToText::setup()
{
    switch (m_state) {
    case Running:
    case Paused:
    case NoPluginFound:
    case PluginError:
    case PermissionMissing:
        return;
    case IncompatibleFormat:
    case NoMicrophone:
        setUpMic();
        break;
    case NotStarted: {
        connect(m_plugin, &SpeechToTextPlugin::loaded, this, &SpeechToText::setUpMic);

        std::ignore = QtConcurrent::run(&SpeechToText::setUpModel, this);
        break;
    }
    }
}

bool SpeechToText::requestMicrophonePermission()
{
#if NEED_MICROPHONE_PERMISSION
    static bool hasPermission = false;
    hasPermission = false;

    QMicrophonePermission microphonePermission;
    switch (qApp->checkPermission(microphonePermission)) {
    case Qt::PermissionStatus::Denied:
    case Qt::PermissionStatus::Undetermined: {
        QEventLoop loop(this);
        qApp->requestPermission(microphonePermission, [this, &loop](const QPermission &permission) {
            hasPermission = (permission.status() == Qt::PermissionStatus::Granted);
            if (!hasPermission)
                setState(PermissionMissing);
            loop.quit();
        });
        loop.exec();
        return hasPermission;
    }
    case Qt::PermissionStatus::Granted:
        return true;
    }
#endif
    Q_UNUSED(this); // For the "`x` can be made static" warning

    return true;
}

bool SpeechToText::hasWord(const QString &word)
{
    if (!m_plugin || !m_plugin->hasLookupSupport())
        return false;

    return m_plugin->canRecognizeWord(word);
}

void SpeechToText::setWakeWord(const QString &word)
{
    if (m_plugin)
        m_plugin->setWakeWord(word);
}
QString SpeechToText::wakeWord()
{
    return m_plugin ? m_plugin->wakeWord() : QLatin1String();
}

QString SpeechToText::language()
{
    if (m_plugin)
        return m_plugin->language();
    else
        return QLatin1String();
}

void SpeechToText::pluginStateChanged()
{
    if (!m_plugin)
        return;

    SpeechToTextPlugin::State s = m_plugin->state();

    if (3 <= s)
        setState(PluginError);
}

void SpeechToText::setState(SpeechToText::State s)
{
    if (m_state == s)
        return;

    m_state = s;

    switch (m_state) {
    case Running:
        m_errorString.clear();
        break;
    case NoPluginFound:
        m_errorString = tr("The app is unable to transcribe speech to text because the necessary "
                           "plugin is missing or could not be loaded.");
        break;
    case PluginError:
        if (m_plugin)
            m_errorString = m_plugin->errorString();
        break;
    case NotStarted:
        m_errorString = tr("The recognizer has not yet been set up");
        break;
    case NoMicrophone:
        m_errorString = tr("No microphone was found or the microphone is not accessible");
        break;
    case IncompatibleFormat:
        m_errorString = tr(
            "The microphone is incompatible with the required audio format (PCM 16bit mono)");
        break;
    case Paused:
        m_errorString = tr("The audio device is closed, and is not processing any audio data");
        break;
    default:
        break;
        m_errorString.clear();
    }

    qDebug().noquote().nospace() << "[debug] SpeechToText state changed: " << s << ": "
                                 << m_errorString;

    Q_EMIT stateChanged();
}

SpeechToText::~SpeechToText() = default;
