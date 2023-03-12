#include "recognizer.h" // Include the header file for the SpeechToText class
#include "global.h"
#include "speechtotext/speechtotextplugin.h"
#include "utils.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QLocale>
#include <QMessageBox>
#include <QPluginLoader>
#include <QThreadPool>

#if NEED_MICROPHONE_PERMISSION
#include <QPermission>
#endif

using namespace utils::literals;

SpeechToText::SpeechToText(const QString &pluginName, QObject *parent)
    : QObject{parent}
{
    const auto staticInstances = QPluginLoader::staticInstances();
    for (QObject *pluginObject : staticInstances) {
        auto *plugin = qobject_cast<SpeechToTextPlugin *>(pluginObject);
        if (!plugin)
            continue;
        m_plugins.append(plugin);
    }

    QDir pluginsDir;
    pluginsDir.setPath(dir::speechToTextPluginDir());

    const auto entryList = pluginsDir.entryList(QDir::Files);

    for (const QString &fileName : entryList) {
        if (!QLibrary::isLibrary(fileName))
            continue;

        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        if (!loader.load()) {
            qWarning() << "Failed to load plugin" << fileName
                       << "due to following reason:" << loader.errorString()
                       << "\nUse `qtplugininfo` for a more advanced error message!";
        }
        QObject *pluginObject = loader.instance();
        if (!pluginObject)
            continue;

        pluginObject->setParent(this);

        SpeechToTextPlugin *plugin = qobject_cast<SpeechToTextPlugin *>(pluginObject);
        if (!plugin)
            continue;

        qInfo() << "Loaded speech to text plugin:" << plugin->pluginName();
        m_plugins.append(plugin);

        if (plugin->pluginName() != pluginName)
            continue;

        m_plugin = plugin;
        connect(m_plugin, &SpeechToTextPlugin::answerReady, this, &SpeechToText::onAnswerReady);
    }

    if (m_plugins.isEmpty())
        qCritical() << "No speech to text plugin found!";
    Q_ASSERT(!m_plugins.isEmpty());

    if (m_plugin == nullptr)
        m_plugin = m_plugins.at(0);

#if NEED_MICROPHONE_PERMISSION
    QMicrophonePermission microphonePermission;
    switch (qApp->checkPermission(microphonePermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(microphonePermission, this, &SpeechToText::setup);
        return;
    case Qt::PermissionStatus::Denied:
        setState(State::NoMicrophone);
        return;
    case Qt::PermissionStatus::Granted:
        setup();
        break; // Proceed
    }
#endif
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
    if (!audio)
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
    if (m_plugin)
        m_plugin->reset();
}

bool SpeechToText::setUpModel()
{
    bool succes = false;
    m_plugin->setup(dir::modelDir(), &succes);
    if (succes)
        Q_EMIT languageChanged();

    return succes;
}

void SpeechToText::setUpMic()
{
    if (m_state != NotStarted || !m_plugin)
        return;

    qDebug() << "[debug] Prepare microphone";

    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
#ifdef QT6
    format.setSampleFormat(QAudioFormat::Int16);

    auto inputDevices = QMediaDevices::audioInputs();
    auto input = QMediaDevices::defaultAudioInput();
#else
    format.setSampleType(QAudioFormat::SignedInt);
    format.setSampleSize(16);

    auto inputDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    auto input = QAudioDeviceInfo::defaultInputDevice();
#endif
    if (inputDevices.isEmpty()) {
        setState(NoMicrophone);
        return;
    } else if (!input.isFormatSupported(format)) {
        setState(IncompatibleFormat);
        return;
    }

    audio = new AUDIOINPUT(format, this);
    connect(audio, &AUDIOINPUT::stateChanged, this, [](QAudio::State state) {
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
        return;
    case IncompatibleFormat:
    case NoMicrophone:
        setUpMic();
        break;
    case ErrorWhileLoading:
    case ModelsMissing:
    case NoModelFound:
    case NotStarted:
        connect(m_plugin,
                &SpeechToTextPlugin::loaded,
                this,
                &SpeechToText::setUpMic,
                Qt::UniqueConnection);

        threading::runFunctionInThreadPool([this] { setUpModel(); });
        break;
    default:
        break;
    }
}

bool SpeechToText::hasWord(const QString &word)
{
    if (!m_plugin || !m_plugin->hasLoopupSupport())
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
    if (m_plugin)
        return m_plugin->wakeWord();
    else
        return QLatin1String();
}

QString SpeechToText::language()
{
    if (m_plugin)
        return m_plugin->language();
    else
        return QLatin1String();
}

void SpeechToText::setState(SpeechToText::State s)
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
    }

    qDebug() << "[debug] SpeechToText state changed:" << s << ":" << m_errorString;

    Q_EMIT stateChanged();
}

SpeechToText::~SpeechToText() {}
