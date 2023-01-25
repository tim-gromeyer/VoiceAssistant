#include "mainwindow.h"
#include "global.h"
#include "modeldownloader.h"
#include "recognizer.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QLineEdit>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QTextToSpeech>
#include <QThreadPool>
#include <QTime>
#include <QTimer>
#include <QWidgetAction>

#include <chrono>
#include <functional>
#include <thread>

using namespace std::chrono_literals;
using namespace literals;

std::shared_ptr<SpeechToText> recognizer;
QSharedPointer<QTextToSpeech> engine;
QHash<QString, QStringList> funcCommandHash;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timeTimer(new QTimer(this))
{
    // Set up UI
    ui->setupUi(this);

    ui->content->setFocus();

    // Set up recognizer
    recognizer.reset(new SpeechToText(this));

    // Set up text to speech
    std::thread(&MainWindow::setupTextToSpeech).detach();

    // Connect the actions
    connect(ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);
    connect(ui->actionOpen_downloader, &QAction::triggered, this, &MainWindow::openModelDownloader);

    // Connect actions
    connect(ui->actionHas_word, &QAction::triggered, this, &MainWindow::onHasWord);

    // Set up time timer
    timeTimer->setInterval(1s);
    connect(timeTimer, &QTimer::timeout, this, &MainWindow::updateTime);
    timeTimer->start();
    updateTime();

    // Set up all (standard) commands
    connect(recognizer.get(),
            &SpeechToText::languageChanged,
            this,
            &MainWindow::setUpCommands,
            Qt::QueuedConnection);

    // Prepare recognizer
    connect(recognizer.get(), &SpeechToText::stateChanged, this, &MainWindow::onStateChanged);
    recognizer->setup();

    connect(recognizer->device(), &Listener::textUpdated, this, &MainWindow::updateText);
    connect(recognizer->device(), &Listener::wakeWord, this, &MainWindow::onWakeWord);
    connect(recognizer->device(), &Listener::doneListening, this, &MainWindow::doneListening);

    connect(ui->action_Quit, &QAction::triggered, this, &MainWindow::confirmQuit);
    connect(ui->muteButton, &QCheckBox::stateChanged, this, &MainWindow::toggleMute);

    setupTrayIcon();
}

void MainWindow::onHelpAbout()
{
    // TODO: Implement about dialog. Don't forget the credits to Vosk and Qt
}

void MainWindow::toggleMute()
{
    ui->content->setFocus();

    if (!recognizer)
        return;

    bool mute = ui->muteButton->isChecked();

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
}

void MainWindow::confirmQuit()
{
    auto ret = QMessageBox::question(this, tr("Quit?"), tr("Do you really want to quit?"));

    if (ret == QMessageBox::Yes)
        QCoreApplication::quit();
}

void MainWindow::setupTextToSpeech()
{
    engine.reset(new QTextToSpeech());
    engine->setLocale(QLocale::system());
}

void MainWindow::setupTrayIcon()
{
    trayIcon.reset(new QSystemTrayIcon(QGuiApplication::windowIcon(), this));
    connect(trayIcon.get(), &QSystemTrayIcon::activated, this, [this] { setVisible(!isVisible()); });

    // TODO: Implement mute action

    auto *menu = new QMenu(this);
    menu->addAction(ui->action_Quit);
    menu->addSeparator();

    trayIcon->setContextMenu(menu);
    trayIcon->show();
}

void MainWindow::onStateChanged()
{
    switch (recognizer->state()) {
    case SpeechToText::NoError:
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
}

void MainWindow::openModelDownloader()
{
    ModelDownloader dia(this);
    dia.exec();
}

void MainWindow::processText(const QString &text)
{
    for (auto &pair : commandAndSlot) {
        for (const auto &command : pair.first) {
            if (text.contains(command)) {
                std::thread(pair.second, text.toStdString()).detach();
                return;
            }
        }
    }

    engine->say(tr("I have not understood this!"));
}

using Plugin = std::string (*)(const std::string &);
using PluginCommand = std::vector<std::string> (*)();

void MainWindow::loadPlugin(const std::string &path)
{
    auto *library = new QLibrary(QString::fromStdString(path));
    if (!library->load()) {
        qDebug() << library->errorString();
        return;
    }

    std::function<std::string(const std::string &)> pluginFunction = (Plugin) library->resolve(
        "onCommand");
    if (!pluginFunction)
        return;

    std::function<std::vector<std::string>()> commandFunction = (PluginCommand) library->resolve(
        "commands");
    if (!commandFunction)
        return;
    auto list = commandFunction();

    QStringList commands;
    commands.reserve((int) list.size());

    for (const auto &command : list)
        commands << QString::fromStdString(command);

    commandAndSlot[commands] = pluginFunction;
}

void MainWindow::setUpCommands()
{
    const QString dir = SpeechToText::dataDir() + STR("/commands/") + recognizer->language();

    QFile jsonFile(dir + STR("/default.json"));
    // open the JSON file
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qDebug() << STR("Failed to open %1\n%2").arg(jsonFile.fileName(), jsonFile.errorString());
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
        // get the function name from the JSON object
        QString funcName = jsonObject[STR("funcName")].toString();
        // get the commands array from the JSON object
        QJsonArray commandsArray = jsonObject[STR("commands")].toArray();
        // convert the commands array to a QStringList
        QStringList commands;
        // reserve memory
        commands.reserve(commandsArray.size());
        for (const auto &command : commandsArray)
            commands << command.toString();

        std::function<void(const std::string &)> func = [this, funcName](const std::string &arg) {
            QMetaObject::invokeMethod(this, funcName.toUtf8().constData(), Q_ARG(std::string, arg));
        };

        // register the function and commands in the commandAndSlot map
        commandAndSlot[commands] = func;

        funcCommandHash[funcName] = commands;
    }
}

QStringList MainWindow::getCommandsForFunction(const QString &func)
{
    return funcCommandHash[func];
}

void MainWindow::say(const QString &text)
{
    if (!engine) {
        qWarning() << "Can not say following text:" << text << "\nTextToSpeech not set up!";
        return;
    }

    engine->say(text);
}

void MainWindow::stop(const std::string &)
{
    if (!engine)
        return;

    engine->stop();
}

void MainWindow::say(const std::string &text)
{
    say(QString::fromStdString(text));
}

void MainWindow::sayTime(const std::string &text)
{
    // Get the current time
    QTime currentTime = QTime::currentTime();

    // Time as string without seconds
    const QString time = QLocale::system().toString(currentTime, QLocale::ShortFormat);

    say(tr("It is %1").arg(time));
}

void MainWindow::repeat(const std::string &_text)
{
    QString text = QString::fromStdString(_text);

    const QStringList commands = getCommandsForFunction(STR("repeat"));

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
        qDebug() << "[debug] Waiting for" << activeThreadCount << "threads to be done.";
        QThreadPool::globalInstance()->waitForDone();
        qDebug() << "[debug] All threads ended";
    }

    engine->deleteLater();
    recognizer->deleteLater();

    engine.reset();
    recognizer.reset();
}
