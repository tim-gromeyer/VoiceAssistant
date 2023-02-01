#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
class QMediaPlayer;
class QSystemTrayIcon;
class QTimer;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow *instance();
    // Ask the user a question and return the answer
    QString ask(const QString &text);

    struct Action
    {
        std::function<void(const std::string &)> func = nullptr;
        QString funcName;
        QStringList responses;

        // Execute program
        QString program;
        QStringList args;

        // Commands it reacts to
        QStringList commands;

        // A sound to play
        QString sound;

        void run(const QString &) const;
    };

public Q_SLOTS:
    static void say(const QString &);
    static void say(const std::string &);

    void playSound(const QString &);

protected:
    void closeEvent(QCloseEvent *) override;

private Q_SLOTS:
    void onStateChanged();

    void toggleTextMode();

    void updateTime();

    void onWakeWord();
    void doneListening();

    void updateText(const QString &);
    static void processText(const QString &);

    void onHelpAbout();

    void onHasWord();

    void openModelDownloader();

    // Setup
    static void setupTextToSpeech();
    void setupTrayIcon();

    void mute(bool mute);
    void toggleMute();

    static QStringList commandsForFuncName(const QString &);

    static void loadCommands();
    static void saveCommands();

    //////////////////////////////////////////////////////////////////
    /// Define new functions here!
    //////////////////////////////////////////////////////////////////

    static void sayTime(const QString &); // Say the current local time
    static void stop(const QString &);    // Stop text to speech
    static void repeat(QString);          // Repeat what the user said

private:
    Ui::MainWindow *ui;

    QAction *muteAction = nullptr;
    QMediaPlayer *player = nullptr;

    // The timer used to display the current time
    QTimer *timeTimer;

    QSharedPointer<QSystemTrayIcon> trayIcon;
};

#endif // MAINWINDOW_H
