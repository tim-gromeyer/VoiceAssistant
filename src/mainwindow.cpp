#include <QLineEdit>
#include <QTime>
#include <QTimer>
#include <QtConcurrentRun>

#include <chrono>

#include "global.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std::chrono_literals;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow), timeTimer(new QTimer(this)), recognizer(new Recognizer(this))
{
    // Set up UI
    ui->setupUi(this);

    connect(ui->actionAbout_Qt, &QAction::triggered, qApp, &QApplication::aboutQt);

    // Disable kaldi debug messages
    vosk_set_log_level(-1);

    // Connect actions
    connect(ui->actionHas_word, &QAction::triggered, this, &MainWindow::onHasWord);

    // Set up time timer
    timeTimer->setInterval(1s);
    connect(timeTimer, &QTimer::timeout, this, &MainWindow::updateTime);
    timeTimer->start();
    updateTime();

    // Prepare recognizer
    connect(recognizer, &Recognizer::stateChanged, this, &MainWindow::onStateChanged);
    auto future = QtConcurrent::run(&Recognizer::setUpModel, recognizer);
    Q_UNUSED(future);
    connect(recognizer->device.get(), &Listener::textUpdated, this, &MainWindow::updateText);
    connect(recognizer->device.get(), &Listener::wakeWord, this, &MainWindow::onWakeWord);
    connect(recognizer->device.get(), &Listener::doneListening, this, &MainWindow::doneListening);
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
        ui->statusLabel->setText(tr("<p>The folder for language models is empty.</p>\n"
                                    "<p>Download some <a href=\"%1\">here</a> and extract the files to the following folder:</p>\n"
                                    "<p><i><a href=\"%2\">%2</a></i>.</p>\n"
                                    "<p>Make sure the folder has one of the following names:</p>\n"
                                    "<p><i>%3</i>.</p>"
                                    ).arg(STR("https://alphacephei.com/vosk/models"), recognizer->modelDir(), QLocale::system().uiLanguages().join(L1(", "))));
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

void MainWindow::onHasWord() {
    QScopedPointer<QDialog> dia(new QDialog(this));
    dia->setWindowTitle(tr("Contains word?"));

    QScopedPointer<QVBoxLayout> layout(new QVBoxLayout(dia.get()));
    dia->setLayout(layout.get());

    QScopedPointer<QLabel> infoLabel(new QLabel(tr("Check if the current language model can (not does) recognize a word."), dia.get()));
    QScopedPointer<QLabel> succesLabel(new QLabel(dia.get()));
    QScopedPointer<QLineEdit> inputLine(new QLineEdit(dia.get()));

    layout->addWidget(infoLabel.get());
    layout->addWidget(succesLabel.get());
    layout->addWidget(inputLine.get());

    infoLabel->setWordWrap(true);

    connect(inputLine.get(), &QLineEdit::textChanged, this, [this, &succesLabel](const QString &word){
        if (recognizer->hasWord(word)) {
            succesLabel->setStyleSheet(STR("color: green"));
            succesLabel->setText(tr("Yes it can!"));
        }
        else {
            succesLabel->setStyleSheet(STR("color: red"));
            succesLabel->setText(tr("No it can't!"));
        }
    });

    dia->exec();
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
