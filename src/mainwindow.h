#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "plugins/base.h"
#include "utils.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
class Jokes;
class PluginBridge;
class QAudioOutput;
class QMediaPlayer;
class QPluginLoader;
class QSettings;
class QSystemTrayIcon;
class QTextToSpeech;
class QTimer;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static MainWindow *instance();

    // Say something and wait to be done
    static void sayAndWait(const QString &);

    // Add a command
    static void addCommand(const actions::Action &);

Q_SIGNALS:
    void muted();

public Q_SLOTS:
    static void say(const QString &);

    // Ask the user a question and return the answer
    static QString ask(const QString &text);

    void bridgeSayAndWait(const QString &);
    void bridgeAsk(const QString &);

    void playSound(const QString &);
    void toggleVisibilty();

    //////////////////////////////////////////////////////////////////
    /// Define new functions here!
    //////////////////////////////////////////////////////////////////

    static void quit();    // Close the app
    static void sayTime(); // Say the current local time
    static void stop();    // Stop text to speech
    static void pause();   // Pause music
    static void resume();  // Resume the playing of music
    static void volumeUp();
    static void volumeDown();
    static void setVolume(int);
    static void setVolume(const QString &);
    static void restart(); // Restart the application

    void tellJoke();

protected:
    void closeEvent(QCloseEvent *) override;

private Q_SLOTS:
    void onSTTStateChanged();
    void onTTSStateChanged();

    void updateTime();

    void onWakeWord();
    void doneListening();

    void updateText(const QString &);
    void processText(const QString &);

    void onHelpAbout();

    void onHasWord();

    void openModelDownloader();
    void openSettings();
    void openCommandWizard();

    // Setup
    static void setupTextToSpeech(const QString &engineName,
                                  const QLocale &language,
                                  const QString &voiceName,
                                  float pitch,
                                  float rate);
    void setupTrayIcon();

    void mute(bool mute);
    void toggleMute();

    void loadCommands();
    void saveCommands();

    void loadPlugins();

    void loadSettings();
    void saveSettings();

private:
    static void applyVolume();
    void firstSetup();

    Ui::MainWindow *ui;

    bool m_muted = false;

    QAction *muteAction = nullptr;
    QMediaPlayer *player = nullptr;
#ifdef QT6
    QAudioOutput *audioOutput = nullptr;
#endif

    // The timer used to display the current time
    QTimer *timeTimer;
    Jokes *jokes;

    PluginBridge *bridge;

    QSystemTrayIcon *trayIcon;

    QSettings *settings = nullptr;

    QList<actions::Action> commands;
};

#endif // MAINWINDOW_H
