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

Q_DECLARE_METATYPE(std::string);

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static void say(const std::string &);
    static void say(const QString &);

private Q_SLOTS:
    void onStateChanged();

    void updateTime();

    void onWakeWord();
    void doneListening();

    void updateText(const QString &);

    void onHelpAbout();

    void onHasWord();

    void openModelDownloader();
    void processText(const QString &);

    // Setup
    static void setupTextToSpeech();
    void setupTrayIcon();

    void confirmQuit();

    void toggleMute();

    static QStringList getCommandsForFunction(const QString &);

    ////////////////////////////////////////////
    /// Define new functions here!
    ////////////////////////////////////////////

    static void sayTime(const std::string &); // Say the current local time
    static void stop(const std::string &);    // Stop text to speech
    static void repeat(const std::string &);  // Repeat what the user said

private:
    void loadPlugin(const std::string &);

    void setUpCommands();

    Ui::MainWindow *ui;

    QTimer *timeTimer;

    std::map<QStringList, std::function<void(const std::string &)>> commandAndSlot;

    QSharedPointer<QSystemTrayIcon> trayIcon;
};

#endif // MAINWINDOW_H
