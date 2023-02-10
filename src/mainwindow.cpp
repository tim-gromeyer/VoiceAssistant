#include "mainwindow.h"
#include "global.h"
#include "modeldownloader.h"
#include "recognizer.h"
#include "ui_mainwindow.h"
#include "utils.h"

#include <QCloseEvent>
#include <QDebug>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QProcess>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QSystemTrayIcon>
#include <QTextToSpeech>
#include <QThreadPool>
#include <QTime>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#endif

#include <chrono>

using namespace std::chrono_literals;
using namespace utils::literals;

SpeechToText *recognizer = nullptr;
QTextToSpeech *engine = nullptr;
QThread *engineThread = new QThread();

QList<MainWindow::Action> commands;
QList<Plugin> plugins;
MainWindow *_instance = nullptr;

float volume = 1.0;

void MainWindow::Action::run(const QString &text) const
{
    if (!funcName.isEmpty()) {
        if (MainWindow::staticMetaObject.indexOfMethod(QString(funcName + L1("()")).toUtf8()) == -1)
            QMetaObject::invokeMethod(_instance,
                                      funcName.toUtf8(),
                                      Qt::QueuedConnection,
                                      Q_ARG(QString, text));
        else
            QMetaObject::invokeMethod(_instance, funcName.toUtf8(), Qt::QueuedConnection);
    }
    if (!responses.isEmpty()) {
        int randomIndex = (int) QRandomGenerator::global()->bounded(responses.size());
        say(responses.at(randomIndex));
    }
    if (!program.isEmpty()) {
        QProcess p(_instance);
        p.setProgram(program);
        p.setArguments(args);
        p.startDetached();
    }
    if (!sound.isEmpty())
        _instance->playSound(sound);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , player(new QMediaPlayer(this))
    , timeTimer(new QTimer(this))
{
    // Set up UI
    ui->setupUi(this);

    ui->content->setFocus();

    // Set _instance
    _instance = this;

    // Set audio output device
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
#endif
    // Set up recognizer
    recognizer = new SpeechToText(this);
    connect(recognizer, &SpeechToText::stateChanged, this, &MainWindow::onSTTStateChanged);
    connect(recognizer,
            &SpeechToText::languageChanged,
            this,
            &MainWindow::loadCommands,
            Qt::QueuedConnection);
#if !NEED_MICROPHONE_PERMISSION
    recognizer->setup();
#endif

    // Set up text to speech
    threading::runFunction(&MainWindow::setupTextToSpeech);

    // Connect the actions
    connect(ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actionHas_word, &QAction::triggered, this, &MainWindow::onHasWord);
    connect(ui->actionOpen_downloader, &QAction::triggered, this, &MainWindow::openModelDownloader);
    connect(ui->action_About, &QAction::triggered, this, &MainWindow::onHelpAbout);

    // Set up time timer
    timeTimer->setInterval(1s);
    connect(timeTimer, &QTimer::timeout, this, &MainWindow::updateTime);
    timeTimer->start();
    updateTime();

    connect(recognizer->device(), &Listener::doneListening, this, &MainWindow::doneListening);
    connect(recognizer->device(), &Listener::textUpdated, this, &MainWindow::updateText);
    connect(recognizer->device(), &Listener::wakeWord, this, &MainWindow::onWakeWord);

    connect(ui->action_Quit, &QAction::triggered, this, &QWidget::close);
    connect(ui->muteButton, &QCheckBox::clicked, this, &MainWindow::mute);

    setupTrayIcon();
}

void MainWindow::addCommand(Plugin a)
{
    plugins.append(a);
    saveCommands();
}

void MainWindow::playSound(const QString &_url)
{
    QUrl url(_url);
    if (QFile::exists(_url))
        url = QUrl::fromLocalFile(_url);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
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
            .arg(STR("beta"), qVersion()));
}

void MainWindow::mute(bool mute)
{
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
}

void MainWindow::toggleMute()
{
    mute(!muteAction->isChecked());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    auto ret = QMessageBox::question(this, tr("Quit?"), tr("Do you really want to quit?"));

    if (ret != QMessageBox::Yes)
        e->ignore();
}

void MainWindow::setupTextToSpeech()
{
    // Use QThread here because otherwise the stateChanged signal doesn't get emitted
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            audioOutput->setVolume(0.3F * volume);
#else
            player->setVolume(int(30 * volume));
#endif
        break;
    case QTextToSpeech::Ready:
        if (player)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
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
    case SpeechToText::NoModelFound:
    case SpeechToText::ModelsMissing:
        ui->statusLabel->setText(recognizer->errorString());
        openModelDownloader();
        break;
    default:
        ui->statusLabel->setText(recognizer->errorString());
    }

    trayIcon->setToolTip(ui->statusLabel->text());
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        audioOutput->setVolume(0.3F * volume);
#else
        player->setVolume(int(30 * volume));
#endif
}

void MainWindow::doneListening()
{
    processText(ui->textLabel->text());
    ui->statusLabel->setText(tr("Waiting for wake word"));
    ui->textLabel->setText({});

    if (player)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        audioOutput->setVolume(1 * volume);
#else
        player->setVolume(int(100 * volume));
#endif
}

void MainWindow::onHasWord()
{
    QDialog dia(this);
    dia.setWindowTitle(tr("Contains word?"));

    auto *layout = new QVBoxLayout(&dia);

    auto *infoLabel
        = new QLabel(tr("Check if the current language model can (not does) recognize a word."),
                     &dia);
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    auto *wordLabel = new QLabel(tr("Word:"), &dia);
    layout->addWidget(wordLabel);

    auto *wordEdit = new QLineEdit(&dia);
    wordEdit->setPlaceholderText(tr("Enter word"));
    layout->addWidget(wordEdit);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, &dia);
    layout->addWidget(buttonBox);

    connect(wordEdit, &QLineEdit::textChanged, wordEdit, [wordEdit](const QString &w) {
        auto word = w.trimmed().toLower();
        if (word.isEmpty()) {
            wordEdit->setStyleSheet(QString());
            return;
        }

        bool hasWord = SpeechToText::hasWord(word);
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

    delete layout;
    delete infoLabel;
    delete wordLabel;
    delete wordEdit;
    delete buttonBox;
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

void MainWindow::processText(const QString &text)
{
    if (text == SpeechToText::wakeWord())
        return;

    for (const auto &action : qAsConst(commands)) {
        for (const auto &command : action.commands) {
            if (text.startsWith(command)) {
                action.run(text.mid(command.length() + 1));
                return;
            }
        }
    }

    engine->say(tr("I have not understood this!"));
}

void MainWindow::loadCommands()
{
    commands.clear();

    const QString dir = SpeechToText::dataDir() + STR("/commands/") + recognizer->language();

    QFile jsonFile(dir + STR("/default.json"));
    // open the JSON file
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qWarning() << STR("Failed to open %1:\n%2")
                          .arg(jsonFile.fileName(), jsonFile.errorString())
                          .toStdString()
                          .c_str();
        return;
    }

    // read all the data from the JSON file
    QByteArray jsonData = jsonFile.readAll();
    jsonFile.close();

    // create a QJsonDocument from the JSON data
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
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
    const QString dir = SpeechToText::dataDir() + STR("/commands/") + recognizer->language();

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
            _instance,
            tr("Failed to save file"),
            tr("Failed to write <em>%1</em>.\n%2\nCopy following text and save it manually:\n%3")
                .arg(jsonFile.fileName(), jsonFile.errorString(), jsonDoc.toJson()));
}

void MainWindow::say(const QString &text)
{
    if (!engine) {
        qWarning() << "Can not say following text:" << text << "\nTextToSpeech not set up!";
        return;
    }

    engine->say(text);
}

void MainWindow::say(const std::string &text)
{
    say(QString::fromStdString(text));
}

void MainWindow::sayAndWait(const QString &text)
{
    if (!engine)
        return;
    QEventLoop loop(_instance);
    connect(engine, &QTextToSpeech::stateChanged, &loop, [&loop](QTextToSpeech::State s) {
        if (s != QTextToSpeech::Speaking)
            loop.quit();
    });
    say(text);
    loop.exec();
}

QString MainWindow::ask(const QString &text)
{
    sayAndWait(text);

    // If this is the case something is definitely wrong
    Q_ASSERT(recognizer->state() != SpeechToText::Running
             || recognizer->state() != SpeechToText::Paused);

    // By making it a pointer we prevent out of scope warnings
    QString answer;

    QEventLoop loop(_instance);
    SpeechToText::reset();
    connect(recognizer, &SpeechToText::answerReady, &loop, &QEventLoop::quit);
    connect(recognizer, &SpeechToText::answerReady, _instance, [&answer](const QString &asw) {
        answer = asw;
    });
    _instance->ui->statusLabel->setText(tr("Waiting for answer..."));
    SpeechToText::ask();
    loop.exec();
    loop.deleteLater();
    _instance->ui->statusLabel->setText(tr("Waiting for wake word"));
    _instance->ui->textLabel->setText({});

    return answer;
}

void MainWindow::stop()
{
    if (engine)
        engine->stop();
    if (_instance->player)
        _instance->player->stop();
}

void MainWindow::pause()
{
    if (_instance->player)
        _instance->player->pause();
}

void MainWindow::resume()
{
    if (_instance->player)
        _instance->player->play();
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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    _instance->audioOutput->setVolume(1.0F * volume);
#else
    if (_instance->player)
        _instance->player->setVolume(int(100 * volume));
#endif

    if (engine)
        engine->setVolume(1.0 * volume);
}

void MainWindow::volumeUp()
{
    if (volume != 1)
        volume += 0.1;

    applyVolume();
}

void MainWindow::volumeDown()
{
    if (volume != 0)
        volume -= 0.1;

    applyVolume();
}

void MainWindow::setVolume(const QString &text)
{
    int volumeInt = utils::wordToNumber(text);

    volume = (float) volumeInt / 10.0F;
    applyVolume();
}

MainWindow::~MainWindow()
{
    delete ui;

    timeTimer->stop();

    int activeThreadCount = QThreadPool::globalInstance()->activeThreadCount();

    if (activeThreadCount != 0) {
        qDebug() << "[debug] Waiting for" << activeThreadCount << "threads to finish.";
        QThreadPool::globalInstance()->waitForDone();
        qDebug() << "[debug] All threads ended";
    }

    recognizer->deleteLater();
    delete recognizer;

    // Prevent deleting the thread while running
    engineThread->quit();
    engineThread->wait();

    if (engine)
        engine->deleteLater();
    engineThread->deleteLater();

    delete engine;
    delete engineThread;
}

// TODO: Let user add commands via GUI
// See: https://doc.qt.io/qt-6/qwizardpage.html,
//      https://doc.qt.io/qt-6/qwizard.html
//      https://doc.qt.io/qt-6/qtwidgets-dialogs-classwizard-example.html
// TODO: Add settings like disabling tray icon, store language and model path and so on
// TODO: Add options for controlling text to speech
// TODO: Load plugins, see https://doc.qt.io/qt-6/qpluginloader.html
// TODO: Implement weather, see Qt weather example
