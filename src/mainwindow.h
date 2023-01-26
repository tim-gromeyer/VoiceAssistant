#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
class QTimer;
class QSystemTrayIcon;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static void say(const std::string &);
    static void say(const QString &);

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

        void run(QObject *, const QString &) const;
    };

private Q_SLOTS:
    void onStateChanged();

    void updateTime();

    void onWakeWord();
    void doneListening();

    void updateText(const QString &);
    void processText(const QString &);

    void onHelpAbout();

    void onHasWord();

    void openModelDownloader();

    // Setup
    static void setupTextToSpeech();
    void setupTrayIcon();

    void confirmQuit();

    void toggleMute();

    static QStringList getCommandsForFunction(const QString &);

    //////////////////////////////////////////////////////////////////
    /// Define new functions here! The function must accept a QString
    //////////////////////////////////////////////////////////////////

    static void sayTime(const QString &); // Say the current local time
    static void stop(const QString &);    // Stop text to speech
    static void repeat(QString);          // Repeat what the user said

private:
    void setUpCommands();

    Ui::MainWindow *ui;

    QTimer *timeTimer;

    QSharedPointer<QSystemTrayIcon> trayIcon;
};

#endif // MAINWINDOW_H
