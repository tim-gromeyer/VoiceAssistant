#include "mainwindow.h"
#include "commands.h"
#include "commandwizard.h"
#include "global.h"
#include "jokes.h"
#include "modeldownloader.h"
#include "plugins/bridge.h"
#include "plugins/utils.h"
#include "recognizer.h"
#include "settingsdialog.h"
#include "speechtotext/speechtotextplugin.h"
#include "texttospeechsettings.h"
#include "ui_mainwindow.h"
#include "utils.h"

#include <QCloseEvent>
#include <QDebug>
#include <QDialogButtonBox>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QLineEdit>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QPluginLoader>
#include <QProcess>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QRegularExpressionMatchIterator>
#include <QSaveFile>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTextToSpeech>
#include <QThreadPool>
#include <QTime>
#include <QTimer>
#include <QWidgetAction>
#include <QtConcurrentRun>

#ifdef QT6
#include <QAudioOutput>
#endif

#include <chrono>

using namespace std::chrono_literals;
using namespace utils::strings::literals;
using actions::Action;

SpeechToText *recognizer = nullptr;
QTextToSpeech *engine = nullptr;
QThread *engineThread = nullptr;

struct Plugin
{
    PluginInterface *interface = nullptr;
    QObject *object = nullptr;
};

QList<Plugin> plugins;
MainWindow *_instance = nullptr;

float volume = 1.0;

void actions::Action::run(QStringView text) const
{
    qDebug().noquote().nospace() << "Running action: " << name;

    if (!funcName.isEmpty()) {
        if (MainWindow::staticMetaObject.indexOfMethod(QString(funcName + L1("()")).toUtf8()) == -1)
            QMetaObject::invokeMethod((QObject *) MainWindow::instance(),
                                      funcName.toUtf8(),
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, text.toString()));
        else
            QMetaObject::invokeMethod((QObject *) MainWindow::instance(),
                                      funcName.toUtf8(),
                                      Qt::QueuedConnection);
    }
    if (!responses.isEmpty()) {
        const int randomIndex = (int) QRandomGenerator::global()->bounded(responses.size());
        MainWindow::say(responses.at(randomIndex));
    }

#ifndef Q_OS_WASM
    if (!program.isEmpty()) {
        QProcess p((QObject *) MainWindow::instance());
        p.setProgram(program);

        auto index = args.indexOf(L1("${TEXT}"));
        if (index != -1) {
            auto newArgs = args; // The function must be const so we need to create a copy
            newArgs[index] = text.toString();
            p.setArguments(newArgs);
        } else {
            qDebug() << "[debug] Starting program:" << program << args;
            p.setArguments(args);
        }

        p.startDetached();
    }
#endif
    if (!sound.isEmpty())
        MainWindow::instance()->playSound(sound);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , player(new QMediaPlayer(this))
    , timeTimer(new QTimer(this))
    , jokes(new Jokes(this))
    , bridge(new PluginBridge(this))
    , trayIcon(new QSystemTrayIcon(qApp->windowIcon(), this))
    , settings(new QSettings(this))
{
    // Set up UI
    ui->setupUi(this);

    ui->content->setFocus();

    // Set instance for instance()
    _instance = this;

    connect(bridge, &PluginBridge::_say, this, &MainWindow::say, Qt::QueuedConnection);
    connect(bridge,
            &PluginBridge::_sayAndWait,
            this,
            &MainWindow::bridgeSayAndWait,
            Qt::QueuedConnection);
    connect(bridge, &PluginBridge::_ask, this, &MainWindow::bridgeAsk, Qt::QueuedConnection);
    connect(
        bridge,
        &PluginBridge::useWidget,
        ui->content,
        [this](QWidget *w) {
            ui->content->takeWidget();
            ui->content->setWidget(w);
        },
        Qt::QueuedConnection);

    // Set audio output device
#ifdef QT6
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
#endif

    // TODO: Detect first startup (after installation) and show a instruction or so
    firstSetup();

    // Set up recognizer
    recognizer = new SpeechToText(STR("vosk"), this);
    connect(recognizer, &SpeechToText::stateChanged, this, &MainWindow::onSTTStateChanged);
    connect(recognizer,
            &SpeechToText::languageChanged,
            this,
            &MainWindow::loadCommands,
            Qt::QueuedConnection);
    if (recognizer->state() != SpeechToText::NotStarted)
        onSTTStateChanged();

    // Set up text to speech
    settings->beginGroup(STR("TextToSpeech"));
    engineThread = new QThread(this);
    connect(engineThread, &QThread::finished, engineThread, &QObject::deleteLater);
    std::ignore = QtConcurrent::run(&MainWindow::setupTextToSpeech,
                                    settings->value(STR("Engine"), QLatin1String()).toString(),
                                    settings->value(STR("Language"), QLocale::system()).toLocale(),
                                    settings->value(STR("Voice"), QLatin1String()).toString(),
                                    settings->value(STR("Pitch"), NAN).toFloat(),
                                    settings->value(STR("Rate"), NAN).toFloat());
    settings->endGroup();

    // Connect the actions
    connect(ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actionHas_word, &QAction::triggered, this, &MainWindow::onHasWord);
    connect(ui->actionOpen_downloader, &QAction::triggered, this, &MainWindow::openModelDownloader);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::openSettings);
    connect(ui->actionAddCommand, &QAction::triggered, this, &MainWindow::openCommandWizard);
    connect(ui->action_About, &QAction::triggered, this, &MainWindow::onHelpAbout);
    connect(ui->volumeSlider, &QSlider::sliderMoved, this, qOverload<int>(&MainWindow::setVolume));
    connect(ui->actionCloseWindow, &QAction::triggered, this, &MainWindow::toggleVisibilty);

    // Set up time timer
    timeTimer->setInterval(1s);
    connect(timeTimer, &QTimer::timeout, this, &MainWindow::updateTime);
    timeTimer->start();
    updateTime();

    if (recognizer->device()) {
        connect(recognizer->device(),
                &SpeechToTextPlugin::doneListening,
                this,
                &MainWindow::doneListening);
        connect(recognizer->device(),
                &SpeechToTextPlugin::textUpdated,
                this,
                &MainWindow::updateText);
        connect(recognizer->device(),
                &SpeechToTextPlugin::wakeWordDetected,
                this,
                &MainWindow::onWakeWord);
    }

    connect(ui->action_Quit, &QAction::triggered, this, &QWidget::close);
    connect(ui->muteButton, &QCheckBox::clicked, this, &MainWindow::mute);

    setupTrayIcon();

    loadPlugins();

    QTimer::singleShot(3s, jokes, &Jokes::setup);

    // Use this otherwise we have a black screen on wasm
    QMetaObject::invokeMethod(
        this,
        [this] {
            if (recognizer->requestMicrophonePermission())
                recognizer->setup();
        },
        Qt::QueuedConnection);

    ui->actionCloseWindow->setShortcuts(QKeySequence::Close);
    ui->action_Quit->setShortcuts(QKeySequence::Quit);
}

void MainWindow::firstSetup()
{
    QDir dir;

    if (dir.exists(dir::commandsBaseDir()))
        return;

    directory::copyRecursively(dir::commandsInstallBaseDir(), dir::commandsBaseDir());
#ifndef QT_DEBUG
    // NOTE: This might fail because on Linux /opt is read-only so we can't delete files/folders or write to files
    dir.remove(dir::commandsInstallBaseDir());
#endif
}

void MainWindow::addCommand(const Action &a)
{
    instance()->commands.append(a);
    instance()->saveCommands();
}

void MainWindow::playSound(const QString &_url)
{
    QUrl url(_url);
    if (QFile::exists(_url))
        url = QUrl::fromLocalFile(_url);

#ifdef QT6
    player->setSource(url);
#else
    player->setMedia(url);
#endif
    player->play();
}

MainWindow *MainWindow::instance()
{
    return _instance;
}

void MainWindow::onHelpAbout()
{
    QMessageBox::about(
        this,
        tr("About VoiceAssistant"),
        tr("<h1>Voice Assistant</h1>\n"
           "<p>A resource-efficient and customizable voice assistant written in c++.</p>\n"
           "<h3>About</h3>\n"
           "<table border=\"0\" style=\"border-collapse: collapse; width: 100%;\">\n"
           "<tbody>\n"
           "<tr>\n"
           "<td style=\"text-align: right; padding-right: 5px;\">Version:</td>\n"
           "<td style=\"text-align: left; padding-left: 5px;\">%1</td>\n"
           "</tr>\n"
           "<tr>\n"
           "<td style=\"text-align: right; padding-right: 5px;\">Qt Version:</td>\n"
           "<td style=\"text-align: left; padding-left: 5px;\">%2</td>\n"
           "</tr>\n"
           "<tr>\n"
           "<td style=\"text-align: right; padding-right: 5px;\">Homepage:</td>\n"
           "<td style=\"text-align: left; padding-left: 5px;\"><a "
           "href=\"https://github.com/tim-gromeyer/VoiceAssistant\">https://github.com/"
           "tim-gromeyer/VoiceAssistant</a></td>\n"
           "</tr>\n"
           "</tbody>\n"
           "</table>\n"
           "<h3>Credits</h3>\n"
           "<p>This project uses <a href=\"https://github.com/alphacep/vosk-api\">Vosk</a> which "
           "is licensed under the <a "
           "href=\"https://github.com/alphacep/vosk-api/blob/master/COPYING\">Apache License "
           "2.0</a>.</p>\n"
           "<p>This project also uses&nbsp;<a href=\"https://github.com/Sygmei/11Zip\" "
           "target=\"_blank\" rel=\"noopener\">11Zip</a>&nbsp;to unpack the downloaded voice "
           "models.</p>")
            .arg(STR(APP_VERSION), qVersion()));
}

void MainWindow::mute(bool mute)
{
    m_muted = mute;
    ui->content->setFocus();

    if (!recognizer)
        return;

    if (mute) {
        ui->muteButton->setText(tr("Unmute"));
        ui->muteButton->setToolTip(tr("Unmute"));
        ui->muteButton->setIcon(QIcon::fromTheme(STR("audio-volume-muted")));
        recognizer->pause();
    } else {
        ui->muteButton->setText(tr("Mute"));
        ui->muteButton->setToolTip(tr("Mute"));
        ui->muteButton->setIcon(QIcon::fromTheme(STR("audio-input-microphone")));
        recognizer->resume();
    }

    muteAction->setText(ui->muteButton->text());
    muteAction->setToolTip(ui->muteButton->toolTip());
    muteAction->setIcon(ui->muteButton->icon());

    muteAction->setChecked(mute);
    ui->muteButton->setChecked(mute);

    Q_EMIT muted();
}

void MainWindow::toggleMute()
{
    mute(!muteAction->isChecked());
}

void MainWindow::toggleVisibilty()
{
    static QPoint position;

    if (isVisible()) {
        position = pos();
        hide();
    } else {
        move(position);
        show();
        ui->content->setFocus();
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    auto ret = QMessageBox::question(this, tr("Quit?"), tr("Do you really want to quit?"));

    if (ret != QMessageBox::Yes) {
        e->ignore();
        return;
    }

    saveSettings();
    e->accept();
    QMainWindow::closeEvent(e);
}

void MainWindow::setupTextToSpeech(const QString &engineName,
                                   const QLocale &language,
                                   const QString &voiceName,
                                   float pitch,
                                   float rate)
{
    qDebug() << "[debug] TTS: Setup QTextToSpeech";
    engine = new QTextToSpeech(engineName.isEmpty() ? QLatin1String() : engineName);
    engine->moveToThread(engineThread);
    engineThread->start();
    engine->setLocale(language);
    const auto availableVoices = engine->availableVoices();
    for (const QVoice &voice : availableVoices) {
        if (voice.name() == voiceName)
            engine->setVoice(voice);
    }
    if (!std::isnan(pitch))
        engine->setPitch(pitch);
    if (!std::isnan(rate))
        engine->setRate(rate);
    connect(engine, &QTextToSpeech::stateChanged, _instance, &MainWindow::onTTSStateChanged);
    qDebug() << "[debug] TTS: Setup finished";
}

void MainWindow::setupTrayIcon()
{
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::toggleVisibilty);

    muteAction = new QAction(this);
    muteAction->setCheckable(true);
    muteAction->setText(tr("Mute"));
    muteAction->setToolTip(tr("Mute"));
    muteAction->setIcon(QIcon::fromTheme(STR("audio-input-microphone")));
    connect(muteAction, &QAction::triggered, this, &MainWindow::mute);

    auto *menu = new QMenu(this);
    menu->addAction(ui->action_Quit);
    menu->addSeparator();
    menu->addAction(muteAction);

    trayIcon->setContextMenu(menu);
    trayIcon->setToolTip(qApp->applicationName());
    trayIcon->show();
}

void MainWindow::onTTSStateChanged()
{
    if (!engine)
        return;

    QTextToSpeech::State s = engine->state();

    switch (s) {
    case QTextToSpeech::Speaking:
        if (player)
#ifdef QT6
            audioOutput->setVolume(0.3F * volume);
#else
            player->setVolume(int(30 * volume));
#endif
        break;
    case QTextToSpeech::Ready:
        if (player)
#ifdef QT6
            audioOutput->setVolume(1 * volume);
#else
            player->setVolume(int(100 * volume));
#endif
        break;
    default:
        break;
    }
}

void MainWindow::onSTTStateChanged()
{
    switch (recognizer->state()) {
    case SpeechToText::Running:
        ui->statusLabel->setText(tr("Waiting for wake word"));
        break;
    case SpeechToText::PermissionMissing: {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(tr("Microphone Access Denied"));
        msgBox.setText(tr("Access to the microphone has been denied. This feature requires "
                          "microphone access to function properly."));
        msgBox.setInformativeText(tr(
            "Please grant microphone access to enable speech-to-text functionality and allow the "
            "app to convert your spoken words into text. Your audio data will only be used for "
            "this purpose and will be handled securely in accordance with our privacy policy."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);

        // Show dialog and wait for user response
        msgBox.exec();
    }
    case SpeechToText::PluginError:
        if (!recognizer->device())
            return;

        ui->statusLabel->setText(recognizer->errorString());
        openModelDownloader();
        break;
    default:
        ui->statusLabel->setText(recognizer->errorString());
    }
}

void MainWindow::updateTime()
{
    ui->timeLabel->setText(QLocale::system().toString(QTime::currentTime()));
}

void MainWindow::updateText(const QString &text)
{
    ui->textLabel->setText(text);
}

void MainWindow::onWakeWord()
{
    ui->statusLabel->setText(tr("Listening …"));

    if (player)
#ifdef QT6
        audioOutput->setVolume(0.3F * volume);
#else
        player->setVolume(int(30 * volume));
#endif
    engine->setVolume(0.3F * volume);
}

void MainWindow::doneListening()
{
    ui->statusLabel->setText(tr("Waiting for wake word"));
    const QString text = ui->textLabel->text();
    ui->textLabel->setText({});
    QMetaObject::invokeMethod(this, "processText", Qt::QueuedConnection, Q_ARG(QString, text));

    if (player)
#ifdef QT6
        audioOutput->setVolume(1.0F * volume);
#else
        player->setVolume(int(100 * volume));
#endif

    engine->setVolume(1.0F * volume);
}

void MainWindow::onHasWord()
{
    QDialog dia(this);
    dia.setWindowTitle(tr("Contains word?"));

    auto *warningLayout = new QVBoxLayout(&dia);
    QMargins m = warningLayout->contentsMargins();
    warningLayout->setContentsMargins(0, 0, 0, 0);

    auto *layout = new QVBoxLayout();
    layout->setContentsMargins(m);

    auto *infoLabel
        = new QLabel(tr("Check if the current language model can (not does) recognize a word."),
                     &dia);
    infoLabel->setWordWrap(true);

    auto *wordLabel = new QLabel(tr("Word:"), &dia);

    auto *wordEdit = new QLineEdit(&dia);
    wordEdit->setPlaceholderText(tr("Enter word"));
    wordEdit->setClearButtonEnabled(true);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dia);

    // Check if the stt plugin supports word lookups
    auto *warningLabel = new QLabel(&dia);
    warningLabel->setStyleSheet(
        STR("background-color: rgb(255, 107, 0); color: black; border-bottom-left-radius: 5px; "
            "border-bottom-right-radius: 5px"));

    if (recognizer->device() && !recognizer->device()->hasLookupSupport())
        warningLabel->setText(tr("Warning: The current STT plugin does not support word lookups"));
    if (recognizer->state() == SpeechToText::NoPluginFound)
        warningLabel->setText(tr("Warning: No STT plugin found!"));
    else if (recognizer->state() == SpeechToText::PluginError)
        warningLabel->setText(tr("Warning: There is a error with the STT plugin."));
    else
        warningLabel->setVisible(false);

    warningLayout->addWidget(warningLabel);
    layout->addWidget(infoLabel);
    layout->addWidget(wordLabel);
    layout->addWidget(wordEdit);
    layout->addWidget(buttonBox);
    warningLayout->addLayout(layout);
    dia.setLayout(warningLayout);

    connect(wordEdit, &QLineEdit::textChanged, wordEdit, [wordEdit](const QString &w) {
        auto word = w.trimmed().toLower();
        if (word.isEmpty()) {
            wordEdit->setStyleSheet(QString());
            return;
        }
        bool hasWord = recognizer->hasWord(word);
        if (hasWord)
            wordEdit->setStyleSheet(STR("color: green"));
        else
            wordEdit->setStyleSheet(STR("color: red"));
    });

    connect(buttonBox, &QDialogButtonBox::accepted, &dia, &QDialog::accept);

    dia.exec();

    dia.deleteLater();
    layout->deleteLater();
    infoLabel->deleteLater();
    wordLabel->deleteLater();
    wordEdit->deleteLater();
    buttonBox->deleteLater();
    warningLabel->deleteLater();

    delete layout;
    delete infoLabel;
    delete wordLabel;
    delete wordEdit;
    delete buttonBox;
    delete warningLabel;
}

void MainWindow::openModelDownloader()
{
    ModelDownloader dia(this);
    connect(
        &dia,
        &ModelDownloader::modelDownloaded,
        this,
        [this] {
            qDebug() << "Model downloaded, reloading model";

            recognizer->setState(SpeechToText::State::NotStarted);

            if (recognizer->requestMicrophonePermission())
                recognizer->setup();
        },
        Qt::QueuedConnection);
    dia.exec();
}

void MainWindow::openSettings()
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    SettingsDialog dia(this);

    TextToSpeechSettings sttSettings(engine, &dia);
    sttSettings.setSettings(settings);
    dia.addSettingsWidget(&sttSettings);
    connect(&sttSettings, &TextToSpeechSettings::setNewTTS, this, [this](QTextToSpeech *tts) {
        qDebug() << "Set new tts engine";
        engine = tts;
    });

    QGuiApplication::restoreOverrideCursor();
    dia.exec();
}

void MainWindow::openCommandWizard()
{
    CommandWizard wizard(bridge, this);
    wizard.exec();

    bool isActionValid = false;
    actions::Action action = wizard.getAction(&isActionValid);

    if (isActionValid)
        addCommand(action);
};

void MainWindow::processText(const QString &text)
{
    using strings::calculateSimilarity;

    // TODO: Make this a setting
    constexpr float SIMILARITY_THRESHOLD = 0.8;

    if (text.isEmpty())
        return;

    // Loop for actions in commands
    for (const auto &action : std::as_const(commands)) {
        for (const auto &command : action.commands) {
            QString commandPrefix = text.left(command.length());
            float similaritry = calculateSimilarity(commandPrefix, command);
            qDebug().noquote().nospace() << "calculateSimilarity(" << commandPrefix << ", "
                                         << command << ") --> " << similaritry;
            if (similaritry >= SIMILARITY_THRESHOLD || text.contains(command)) {
                QString parameter = text.mid(commandPrefix.length() + 1);
                action.run(parameter);
                return;
            }
        }
    }

    // Loop for plugins
    for (const Plugin &plugin : std::as_const(plugins)) {
        if (!plugin.interface->isValid(text))
            continue;

        plugin.interface->run(text);
        return;
    }

    // Error handling: No matching action found
    say(tr("I have not understood this!"));
}

void MainWindow::loadCommands()
{
    commands = Commands::getActions(recognizer->language());
}

void MainWindow::saveCommands()
{
    Commands::saveActions(commands, recognizer->language());
}

void MainWindow::loadPlugins()
{
    const auto staticInstances = QPluginLoader::staticInstances();
    for (QObject *pluginObject : staticInstances) {
        auto *interface = qobject_cast<PluginInterface *>(pluginObject);
        if (!interface)
            continue;

        interface->setBridge(bridge);

        Plugin plugin;
        plugin.interface = interface;
        plugin.object = pluginObject;

        plugins.append(plugin);
    }

    QDir pluginsDir;
    pluginsDir.setPath(dir::pluginDir());

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

        PluginInterface *interface = qobject_cast<PluginInterface *>(pluginObject);
        if (!interface)
            continue;

        interface->setBridge(bridge);
        Plugin plugin;
        plugin.interface = interface;
        plugin.object = pluginObject;

        qInfo() << "Loaded plugin:" << loader.fileName();
        plugins.append(plugin);
    }
}

void MainWindow::loadSettings() {}

void MainWindow::saveSettings() {}

//void MainWindow::say(const QString &text)
//{
//    if (!engine) {
//        qWarning() << "Can not say following text:" << text << "\nTextToSpeech not set up!";
//        return;
//    }

//    engine->say(text);
//}
void MainWindow::say(const QString &text)
{
    if (!engine) {
        qWarning() << "Can not say following text:" << text << "\nTextToSpeech not set up!";
        return;
    }

    static QRegularExpression regex(QStringLiteral("\\[wait (\\d+)\\]"));
    QStringList tokens = text.split(regex);
    QList<int> waitTimes;
    QRegularExpressionMatchIterator it = regex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int waitTime = match.captured(1).toInt();
        waitTimes.append(waitTime);
    }

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    for (int i = 0; i < tokens.size(); ++i) {
        sayAndWait(tokens[i]);

        if (i < waitTimes.size()) {
            timer.start(waitTimes[i]);
            loop.exec();
        }
    }
}

void MainWindow::sayAndWait(const QString &text)
{
    if (!engine || text.isEmpty())
        return;

    QEventLoop loop(instance());
    connect(
        engine,
        &QTextToSpeech::stateChanged,
        &loop,
        [&loop](QTextToSpeech::State s) {
            if (s != QTextToSpeech::Speaking)
                loop.quit();
        },
        Qt::QueuedConnection);
    engine->say(text);
    loop.exec();
}

QString MainWindow::ask(const QString &text)
{
    if (instance()->m_muted)
        return {QLatin1String()};
    else if (recognizer->state() != SpeechToText::Running)
        return recognizer->errorString();

    sayAndWait(text);

    // If this is the case something is definitely wrong
    Q_ASSERT(recognizer->state() != SpeechToText::Running
             || recognizer->state() != SpeechToText::Paused);

    // By making it a pointer we prevent out of scope warnings
    static QString answer;
    answer.clear();

    QEventLoop loop;

    connect(recognizer, &SpeechToText::answerReady, &loop, [&loop](const QString &asw) {
        answer = asw;
    });
    connect(recognizer, &SpeechToText::answerReady, &loop, &QEventLoop::quit, Qt::QueuedConnection);

    if (instance()->m_muted)
        return {QLatin1String()};

    recognizer->ask();
    recognizer->reset();
    instance()->ui->statusLabel->setText(tr("Waiting for answer..."));
    loop.exec();
    instance()->ui->statusLabel->setText(tr("Waiting for wake word"));
    instance()->ui->textLabel->setText({});

    return answer;
}

void MainWindow::bridgeSayAndWait(const QString &text)
{
    sayAndWait(text);
    bridge->pause = false;
}

void MainWindow::bridgeAsk(const QString &text)
{
    if (!m_muted)
        recognizer->resume();

    bridge->answer = ask(text);
    bridge->pause = false;
}

void MainWindow::stop()
{
    if (engine)
        engine->stop();
    if (instance()->player)
        instance()->player->stop();
}

void MainWindow::pause()
{
    if (instance()->player)
        instance()->player->pause();
}

void MainWindow::resume()
{
    if (instance()->player)
        instance()->player->play();
}

void MainWindow::quit()
{
    const QString answer = ask(tr("Are you sure?"));

    if (answer.startsWith(tr("yes"))) {
        sayAndWait(tr("Okay"));
        qApp->quit();
    }
}

void MainWindow::sayTime()
{
    // Get the current time
    QTime currentTime = QTime::currentTime();

    // Time as string without seconds
    const QString time = QLocale::system().toString(currentTime, QLocale::ShortFormat);

    say(tr("It is %1").arg(time));
}

void MainWindow::applyVolume()
{
    qDebug() << "[debug] Change volume to:" << volume;
#ifdef QT6
    instance()->audioOutput->setVolume(1.0F * volume);
#else
    if (instance()->player)
        instance()->player->setVolume(int(100 * volume));
#endif

    if (engine)
        engine->setVolume(1.0 * volume);
}

void MainWindow::volumeUp()
{
    if (volume != 1)
        volume += 0.1F;

    applyVolume();
}

void MainWindow::volumeDown()
{
    if (volume != 0)
        volume -= 0.1F;

    applyVolume();
}

void MainWindow::setVolume(int volumeInt)
{
    volume = (float) volumeInt / 10.0F;
    applyVolume();
}

void MainWindow::setVolume(const QString &text)
{
    int volumeInt = utils::wordToNumber(text);
    instance()->ui->volumeSlider->setValue(volumeInt);
    setVolume(volumeInt);
}

void MainWindow::tellJoke()
{
    jokes->tellJoke();
}

void MainWindow::restart()
{
    const QString answer = ask(tr("Are you sure?"));

    if (!answer.startsWith(tr("yes")))
        return;

    sayAndWait(tr("Okay"));

    QStringList args = qApp->arguments();
    args.removeFirst();
#ifndef Q_OS_WASM
    QProcess::startDetached(qApp->applicationFilePath(), args);
#endif
    qApp->quit();
}

MainWindow::~MainWindow()
{
    timeTimer->stop();
    timeTimer->deleteLater();

    delete ui;

    int activeThreadCount = QThreadPool::globalInstance()->activeThreadCount();

    if (activeThreadCount != 0) {
        qDebug() << "[debug] Waiting for" << activeThreadCount << "threads to finish.";
        QThreadPool::globalInstance()->waitForDone();
        qDebug() << "[debug] All threads ended";
    }

    recognizer->deleteLater();
    delete recognizer;

    bridge->deleteLater();
    delete bridge;

    jokes->deleteLater();

    // Prevent deleting the thread while running
    if (engineThread) {
        engineThread->quit();
        engineThread->wait();
    }

    if (engine)
        engine->deleteLater();

    delete engine;
    delete engineThread;
    delete jokes;
    delete timeTimer;
}

// TODO: Add settings like disabling tray icon, store language and model path and so on
// TODO: Implement weather as a plugin(so it's easier to exclude), see Qt weather example
