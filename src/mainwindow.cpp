#include "mainwindow.h"
#include "global.h"
#include "modeldownloader.h"
#include "recognizer.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QDebug>
#include <QDialogButtonBox>
#include <QGraphicsDropShadowEffect>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QProcess>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QSystemTrayIcon>
#include <QTextToSpeech>
#include <QThreadPool>
#include <QTime>
#include <QTimer>

#include <chrono>
#include <functional>
#include <thread>

using namespace std::chrono_literals;
using namespace literals;

SpeechToText *recognizer;
QTextToSpeech *engine;

QList<MainWindow::Action> commands;
MainWindow *_instance = nullptr;

void MainWindow::Action::run(const QString &text) const
{
    if (func) {
        func(text.toStdString());
        return;

    } else if (!funcName.isEmpty()) {
        QMetaObject::invokeMethod(_instance, funcName.toUtf8(), Q_ARG(QString, text));
    } else if (!responses.isEmpty()) {
        int randomIndex = QRandomGenerator::global()->bounded(responses.size());
        say(responses.at(randomIndex));
    } else if (!program.isEmpty()) {
        QProcess p(_instance);
        p.setProgram(program);
        p.setArguments(args);
        p.startDetached();
    } else if (!sound.isEmpty())
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

    // Set up recognizer
    recognizer = new SpeechToText(this);

    // Set up text to speech
    std::thread(&MainWindow::setupTextToSpeech).detach();

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

    // Set up all commands
    connect(recognizer,
            &SpeechToText::languageChanged,
            this,
            &MainWindow::loadCommands,
            Qt::QueuedConnection);

    // Prepare recognizer
    connect(recognizer, &SpeechToText::stateChanged, this, &MainWindow::onStateChanged);
    recognizer->setup();

    connect(recognizer->device(), &Listener::doneListening, this, &MainWindow::doneListening);
    connect(recognizer->device(), &Listener::textUpdated, this, &MainWindow::updateText);
    connect(recognizer->device(), &Listener::wakeWord, this, &MainWindow::onWakeWord);

    connect(ui->action_Quit, &QAction::triggered, this, &QWidget::close);
    connect(ui->muteButton, &QCheckBox::stateChanged, this, &MainWindow::toggleMute);

    setupTrayIcon();
}

void MainWindow::playSound(const QString &url)
{
    player->setMedia(QUrl(url));
    player->play();
}

MainWindow *instance()
{
    return _instance;
}

void MainWindow::onHelpAbout()
{
    // TODO: Finish about dialog
    QMessageBox::about(
        this,
        tr("About VoiceAssistant"),
        tr("<h1>Voice Assistant</h1>\n"
           "<p>A resource-efficient and customizable voice assistant written in c++.</p>\n"
           "<h1>About</h1>\n"
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
           "<h1>Credits</h1>\n"
           "<p>This project uses <a href=\"https://github.com/alphacep/vosk-api\">Vosk</a> which "
           "is licensed under the <a "
           "href=\"https://github.com/alphacep/vosk-api/blob/master/COPYING\">Apache License "
           "2.0</a>.</p>\n"
           "<p>This project also uses&nbsp;<a href=\"https://github.com/Sygmei/11Zip\" "
           "target=\"_blank\" rel=\"noopener\">11Zip</a>&nbsp;to unpack the downloaded voice "
           "modells.</p>")
            .arg(STR("beta"), qVersion()));
}

void MainWindow::toggleMute(bool mute)
{
    qDebug() << __func__ << mute;
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

void MainWindow::closeEvent(QCloseEvent *e)
{
    auto ret = QMessageBox::question(this, tr("Quit?"), tr("Do you really want to quit?"));

    if (ret != QMessageBox::Yes)
        e->ignore();
}

void MainWindow::setupTextToSpeech()
{
    engine = new QTextToSpeech();
    engine->setLocale(QLocale::system()); // WARNING: This could fail!
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
    connect(muteAction, &QAction::triggered, this, &MainWindow::toggleMute);

    auto *menu = new QMenu(this);
    menu->addAction(ui->action_Quit);
    menu->addSeparator();
    menu->addAction(muteAction);

    trayIcon->setContextMenu(menu);
    trayIcon->show();
}

void MainWindow::onStateChanged()
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
}

void MainWindow::doneListening()
{
    processText(ui->textLabel->text());
    ui->statusLabel->setText(tr("Waiting for wake word"));
    ui->textLabel->setText({});
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
    dia.exec();
}

void MainWindow::processText(const QString &text)
{
    for (const auto &action : qAsConst(commands)) {
        for (const auto &command : action.commands) {
            if (text.contains(command)) {
                std::thread(&Action::run, action, text).detach();
                return;
            }
        }
    }

    engine->say(tr("I have not understood this!"));
}

void MainWindow::loadCommands()
{
    const QString dir = SpeechToText::dataDir() + STR("/commands/") + recognizer->language();

    QFile jsonFile(dir + STR("/default.json"));
    // open the JSON file
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qDebug() << STR("Failed to open %1:\n%2")
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

        jsonObject[STR("funcName")] = c.funcName;
        jsonObject[STR("program")] = c.program;

        QJsonArray commandsArray;
        for (const auto &command : c.commands)
            commandsArray.append(command);
        jsonObject[STR("commands")] = commandsArray;

        QJsonArray responseArray;
        for (const auto &response : c.responses)
            responseArray.append(response);
        jsonObject[STR("responses")] = responseArray;

        QJsonArray argsArray;
        for (const auto &arg : c.args)
            argsArray.append(arg);
        jsonObject[STR("args")] = argsArray;

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
            this,
            tr("Failed to save file"),
            tr("Failed to write <em>%1</em>.\n%2\nCopy following text and save it manually:\n%3")
                .arg(jsonFile.fileName(), jsonFile.errorString(), jsonDoc.toJson()));
}

QStringList MainWindow::commandsForFuncName(const QString &funcName)
{
    auto it = std::find_if(commands.begin(), commands.end(), [&funcName](const Action &action) {
        return action.funcName == funcName;
    });

    if (it == commands.end())
        return {};

    return it->commands;
}

void MainWindow::say(const QString &text)
{
    // TODO: Make music (player) silenter while speaking
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

void MainWindow::stop(const QString &)
{
    if (engine && engine->state() == QTextToSpeech::Speaking)
        engine->stop();
    else
        _instance->player->stop();
}

void MainWindow::sayTime(const QString &text)
{
    // Get the current time
    QTime currentTime = QTime::currentTime();

    // Time as string without seconds
    const QString time = QLocale::system().toString(currentTime, QLocale::ShortFormat);

    say(tr("It is %1").arg(time));
}

void MainWindow::repeat(QString text)
{
    const QStringList commands = commandsForFuncName(STR("repeat"));

    for (const auto &command : commands) {
        if (!text.startsWith(command))
            continue;

        text.remove(0, command.length() + 1); // +1 for space after the command
    }

    say(text);
}

MainWindow::~MainWindow()
{
    delete ui;

    int activeThreadCount = QThreadPool::globalInstance()->activeThreadCount();

    if (activeThreadCount != 0) {
        qDebug() << "[debug] Waiting for" << activeThreadCount << "threads to finish.";
        QThreadPool::globalInstance()->waitForDone();
        qDebug() << "[debug] All threads ended";
    }

    engine->deleteLater();
    recognizer->deleteLater();

    delete engine;
    delete recognizer;
}

// TODO: Let user add commands via GUI
// TODO: Add settings like disabling tray icon, store language and model path and so on
// TODO: Add text input mode if speech to text doesn't work for some reason
// TODO: Add options for controlling text to speech
