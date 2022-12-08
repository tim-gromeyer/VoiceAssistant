#include <QTime>
#include <QTimer>
#include <QtConcurrentRun>

#include <chrono>

#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std::chrono_literals;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    // Set up UI
    ui->setupUi(this);

    connect(ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);

    // Disable kaldi debug messages
    vosk_set_log_level(-1);

    // Set up time timer
    timeTimer = new QTimer(this);
    timeTimer->setInterval(1s);
    connect(timeTimer, &QTimer::timeout, this, &MainWindow::updateTime);
    timeTimer->start();
    updateTime();

    // Prepare recognizer
    recognizer = new Recognizer(this);
    connect(recognizer, &Recognizer::stateChanged, this, &MainWindow::onStateChanged);
    auto future = QtConcurrent::run(&Recognizer::setUpModel, recognizer);
    Q_UNUSED(future);
    connect(recognizer->device, &Listener::textUpdated, this, &MainWindow::updateText);
    connect(recognizer->device, &Listener::wakeWord, this, &MainWindow::onWakeWord);
    connect(recognizer->device, &Listener::doneListening, this, &MainWindow::doneListening);
}

void MainWindow::onStateChanged(Recognizer::State state) {
    switch (state) {
    case Recognizer::Ok:
        ui->statusLabel->setText(tr("Waiting for wake word"));
        break;
    case Recognizer::ErrorWhileLoading:
        ui->statusLabel->setText(tr("The voice recognition setup failed :("));
        break;
    case Recognizer::ModelsMissing:
        ui->statusLabel->setText(tr("The folder for language models is empty. Download some from the Models menu."));
        break;
    case Recognizer::NoModelFound:
        ui->statusLabel->setText(tr("No language model found for your system language."));
        break;
    default:
        break;
    }

    recognizer->setUpMic();
}

void MainWindow::updateTime() {
    static const QLocale sys = QLocale::system();

    ui->timeLabel->setText(sys.toString(QTime::currentTime()));
}

void MainWindow::updateText(const QString &text) {
    ui->textLabel->setText(text);
}

void MainWindow::onWakeWord() {
    ui->statusLabel->setText(tr("Listening ..."));
}

void MainWindow::doneListening() {
    ui->statusLabel->setText(tr("Waiting for wake word"));
    ui->textLabel->setText(QString());
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

    recognizer->deleteLater();

    delete recognizer;
}
