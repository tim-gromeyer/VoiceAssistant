#include "recognizer.h" // Include the header file for the SpeechToText class
#include "global.h"
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

        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName), this);
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
    }

    if (m_plugins.isEmpty())
        qCritical() << "No speech to text plugin found!";
    Q_ASSERT(!m_plugins.isEmpty());

    if (m_plugin == nullptr)
        m_plugin = m_plugins.at(0);

    connect(m_plugin, &SpeechToTextPlugin::answerReady, this, &SpeechToText::onAnswerReady);
    connect(m_plugin, &SpeechToTextPlugin::stateChanged, this, &SpeechToText::pluginStateChanged);

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
    case NotStarted: {
        connect(m_plugin,
                &SpeechToTextPlugin::loaded,
                this,
                &SpeechToText::setUpMic,
                Qt::UniqueConnection);

#if QT5
        QtConcurrent::run(this, &SpeechToText::setUpModel);
#else
        std::ignore = QtConcurrent::run(&SpeechToText::setUpModel, this);
#endif
        break;
    }
    default:
        break;
    }
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
    m_state = s;

    switch (m_state) {
    case Running:
        m_errorString.clear();
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
    }

    qDebug() << "[debug] SpeechToText state changed:" << s << ":" << m_errorString;

    Q_EMIT stateChanged();
}

SpeechToText::~SpeechToText() {}
