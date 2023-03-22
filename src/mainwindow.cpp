#include "mainwindow.h"
#include "global.h"
#include "jokes.h"
#include "modeldownloader.h"
#include "plugins/bridge.h"
#include "recognizer.h"
#include "settingsdialog.h"
#include "speechtotext/speechtotextplugin.h"
#include "speechtotextsettings.h"
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
#include <QSaveFile>
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
using namespace utils::literals;

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

void MainWindow::Action::run(const QString &text) const
{
    if (!funcName.isEmpty()) {
        if (MainWindow::staticMetaObject.indexOfMethod(QString(funcName + L1("()")).toUtf8()) == -1)
            QMetaObject::invokeMethod(instance(),
                                      funcName.toUtf8(),
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, text));
        else
            QMetaObject::invokeMethod(instance(), funcName.toUtf8(), Qt::QueuedConnection);
    }
    if (!responses.isEmpty()) {
        int randomIndex = (int) QRandomGenerator::global()->bounded(responses.size());
        say(responses.at(randomIndex));
    }

    if (!program.isEmpty()) {
        QProcess p(instance());
        p.setProgram(program);

        auto index = args.indexOf(L1("${TEXT}"));
        if (index != -1) {
            auto newArgs = args; // The function must be const so we need to create a copy
            newArgs[index] = text;
            p.setArguments(newArgs);
        } else
            p.setArguments(args);

        p.startDetached();
    }
    if (!sound.isEmpty())
        instance()->playSound(sound);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , player(new QMediaPlayer(this))
    , timeTimer(new QTimer(this))
    , jokes(new Jokes(this))
    , bridge(new PluginBridge(this))
{
    // Set up UI
    ui->setupUi(this);

    ui->content->setFocus();

    // Set instance for instance()
    _instance = this;

    // Set audio output device
#ifdef QT6
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
#endif

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
#if !NEED_MICROPHONE_PERMISSION
    recognizer->setup();
#endif

    // Set up text to speech
    engineThread = new QThread(this);
    connect(engineThread, &QThread::finished, engineThread, &QObject::deleteLater);
    std::ignore = QtConcurrent::run(&MainWindow::setupTextToSpeech);

    // Connect the actions
    connect(ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actionHas_word, &QAction::triggered, this, &MainWindow::onHasWord);
    connect(ui->actionOpen_downloader, &QAction::triggered, this, &MainWindow::openModelDownloader);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::openSettings);
    connect(ui->action_About, &QAction::triggered, this, &MainWindow::onHelpAbout);
    connect(ui->volumeSlider, &QSlider::sliderMoved, this, qOverload<int>(&MainWindow::setVolume));

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

    //        auto *slider = new SliderWithText(this);
    //        slider->setOrientation(Qt::Horizontal);
    //        slider->setRange(0, 10);
    //        auto *a = new QWidgetAction(this);
    //        a->setDefaultWidget(slider);
    //        ui->menuCommands->addAction(a);
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
           "modells.</p>")
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
    setVisible(!isVisible());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    auto ret = QMessageBox::question(this, tr("Quit?"), tr("Do you really want to quit?"));

    if (ret != QMessageBox::Yes)
        e->ignore();
}

void MainWindow::setupTextToSpeech()
{
    qDebug() << "[debug] TTS: Setup QTextToSpeech";
    engine = new QTextToSpeech();
    engine->moveToThread(engineThread);
    engineThread->start();
    engine->setLocale(QLocale::system()); // WARNING: This might fail!
    connect(engine, &QTextToSpeech::stateChanged, _instance, &MainWindow::onTTSStateChanged);
    qDebug() << "[debug] TTS: Setup finished";
}

void MainWindow::setupTrayIcon()
{
    trayIcon.reset(new QSystemTrayIcon(QGuiApplication::windowIcon(), this));
    connect(trayIcon.get(), &QSystemTrayIcon::activated, this, [this] { setVisible(!isVisible()); });

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
    case SpeechToText::PluginError:
        if (!recognizer->device())
            return;

        if (recognizer->device()->state() != SpeechToTextPlugin::NoModelFound
            && recognizer->device()->state() != SpeechToTextPlugin::ModelsMissing)
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
    ui->statusLabel->setText(tr("Listening ..."));

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
    auto *warningLabel
        = new QLabel(tr("Warning: the current STT plugin does not support word lookups"), &dia);
    warningLabel->setStyleSheet(
        STR("background-color: rgb(255, 107, 0); color: black; border-bottom-left-radius: 5px; "
            "border-bottom-right-radius: 5px"));
    warningLabel->setVisible(recognizer->device() && !recognizer->device()->hasLookupSupport());

    warningLayout->addWidget(warningLabel);
    layout->addWidget(infoLabel);
    layout->addWidget(wordLabel);
    layout->addWidget(wordEdit);
    layout->addWidget(buttonBox);
    warningLayout->addLayout(layout);
    dia.setLayout(warningLayout);

    connect(wordEdit, &QLineEdit::textChanged, wordEdit, [wordEdit, warningLabel](const QString &w) {
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
    connect(&dia,
            &ModelDownloader::modelDownloaded,
            recognizer,
            &SpeechToText::setup,
            Qt::QueuedConnection);
    dia.exec();
}

void MainWindow::openSettings()
{
    SettingsDialog dia(this);
    SpeechToTextSettings sttSettings(engine, &dia);
    dia.addSettingsWidget(&sttSettings);
    dia.exec();
}

void MainWindow::processText(const QString &text)
{
    if (text.isEmpty())
        return;

    for (const auto &action : qAsConst(commands)) {
        for (const auto &command : action.commands) {
            if (text.startsWith(command)) {
                action.run(text.mid(command.length() + 1));
                return;
            }
        }
    }

    for (const Plugin &plugin : qAsConst(plugins)) {
        if (!plugin.interface->isValid(text))
            continue;

        plugin.interface->run(text);
        return;
    }

    say(tr("I have not understood this!"));
}

void MainWindow::loadCommands()
{
    commands.clear();

    const QString dir = dir::commandsBaseDir() + recognizer->language();

    QFile jsonFile(dir + STR("/default.json"));
    // open the JSON file
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qCritical() << STR("Failed to open %1:\n%2")
                           .arg(jsonFile.fileName(), jsonFile.errorString())
                           .toStdString()
                           .c_str();
        return;
    }

    // read all the data from the JSON file
    QByteArray jsonData = jsonFile.readAll();
    jsonFile.close();

    // In case of an error
    QJsonParseError error{};

    // create a QJsonDocument from the JSON data
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &error);

    // Error handling
    if (error.error != QJsonParseError::NoError) {
        qCritical() << STR("JSON parsing error at %1: %2")
                           .arg(QString::number(error.offset), error.errorString());
        return;
    }

    // get the array from the JSON document
    QJsonArray jsonArray = jsonDoc.array();

    for (const auto &value : jsonArray) {
        // convert the array element to an object
        QJsonObject jsonObject = value.toObject();

        // get the commands array from the JSON object
        QJsonArray commandsArray = jsonObject[STR("commands")].toArray();
        if (commandsArray.isEmpty()) {
            qWarning() << "No commands specified for:" << jsonObject;
            continue;
        }

        // Command template
        Action c;

        // get the function name from the JSON object(optional)
        c.funcName = jsonObject[STR("funcName")].toString();

        // get the responses(optional)
        QJsonArray responseArray = jsonObject[STR("responses")].toArray();
        for (const auto &response : responseArray)
            c.responses.append(response.toString());

        // sound/song to play(optional)
        c.sound = jsonObject[STR("sound")].toString();

        // used to execute external programs(optional)
        c.program = jsonObject[STR("program")].toString();
        QJsonArray args = jsonObject[STR("args")].toArray();
        c.args.reserve(args.size());
        for (const auto &arg : args)
            c.args.append(arg.toString());

        // reserve memory
        c.commands.reserve(commandsArray.size());
        for (const auto &command : commandsArray)
            c.commands << command.toString();

        commands.append(c);
    }
}

void MainWindow::saveCommands()
{
    const QString dir = dir::commandsBaseDir() + recognizer->language();

    QJsonArray jsonArray;

    for (const auto &c : qAsConst(commands)) {
        QJsonObject jsonObject;

        if (!c.funcName.isEmpty())
            jsonObject[STR("funcName")] = c.funcName;
        if (!c.program.isEmpty())
            jsonObject[STR("program")] = c.program;
        if (!c.sound.isEmpty())
            jsonObject[STR("sound")] = c.sound;

        if (!c.commands.isEmpty()) {
            QJsonArray commandsArray;
            for (const auto &command : c.commands)
                commandsArray.append(command);
            jsonObject[STR("commands")] = commandsArray;
        }

        if (!c.responses.isEmpty()) {
            QJsonArray responseArray;
            for (const auto &response : c.responses)
                responseArray.append(response);
            jsonObject[STR("responses")] = responseArray;
        }

        if (!c.args.isEmpty()) {
            QJsonArray argsArray;
            for (const auto &arg : c.args)
                argsArray.append(arg);
            jsonObject[STR("args")] = argsArray;
        }
        if (!jsonObject.isEmpty())
            jsonArray.append(jsonObject);
    }

    QJsonDocument jsonDoc(jsonArray);

    QSaveFile jsonFile(dir + STR("/default.json"));
    if (!jsonFile.open(QIODevice::WriteOnly)) {
        qDebug() << STR("Failed to open %1\n%2").arg(jsonFile.fileName(), jsonFile.errorString());
        return;
    }
    jsonFile.write(jsonDoc.toJson());
    if (!jsonFile.commit())
        QMessageBox::warning(
            instance(),
            tr("Failed to save file"),
            tr("Failed to write <em>%1</em>.\n%2\nCopy following text and save it manually:\n%3")
                .arg(jsonFile.fileName(), jsonFile.errorString(), jsonDoc.toJson()));
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

    if (plugins.isEmpty())
        return;

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
}

void MainWindow::say(const QString &text)
{
    if (!engine) {
        qWarning() << "Can not say following text:" << text << "\nTextToSpeech not set up!";
        return;
    }

    engine->say(text);
}

void MainWindow::sayAndWait(const QString &text)
{
    if (!engine)
        return;
    QEventLoop loop(instance());
    connect(engine, &QTextToSpeech::stateChanged, &loop, [&loop](QTextToSpeech::State s) {
        if (s != QTextToSpeech::Speaking)
            loop.quit();
    });
    say(text);
    loop.exec();
}

QString MainWindow::ask(const QString &text)
{
    if (instance()->m_muted)
        return {QLatin1String()};

    sayAndWait(text);

    // If this is the case something is definitely wrong
    Q_ASSERT(recognizer->state() != SpeechToText::Running
             || recognizer->state() != SpeechToText::Paused);

    // By making it a pointer we prevent out of scope warnings
    static QString answer;
    answer.clear();

    QEventLoop loop;

    recognizer->reset();
    connect(recognizer, &SpeechToText::answerReady, &loop, [&loop](const QString &asw) {
        answer = asw;
    });
    connect(recognizer, &SpeechToText::answerReady, &loop, &QEventLoop::quit);

    if (instance()->m_muted)
        return {QLatin1String()};
    recognizer->ask();
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
    QProcess::startDetached(qApp->applicationFilePath(), args);
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

/* TODO: Let user add commands via GUI, 
 * see: https://doc.qt.io/qt-6/qwizardpage.html,
 *      https://doc.qt.io/qt-6/qwizard.html
 *      https://doc.qt.io/qt-6/qtwidgets-dialogs-classwizard-example.html */
// TODO: Add settings like disabling tray icon, store language and model path and so on
// TODO: Add options for controlling text to speech
// TODO: Implement weather as a plugin(so it's easier to exclude), see Qt weather example
