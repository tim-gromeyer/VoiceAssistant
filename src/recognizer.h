#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include "speechtotext/speechtotextplugin.h"

#include <QIODevice>

#ifdef QT6
#include <QAudioSource>
#include <QMediaDevices>
#define AUDIOINPUT QAudioSource
#else
#include <QAudioDeviceInfo>
#include <QAudioInput>
#define AUDIOINPUT QAudioInput
#endif

class SpeechToText : public QObject
{
    Q_OBJECT

    friend class MainWindow;

public:
    explicit SpeechToText(const QString & = QStringLiteral("vosk"), QObject *parent = nullptr);
    ~SpeechToText();

    enum State {
        NoPluginFound = 0,      // No speech to text plugin could be found or loaded
        PluginError = 1,        // An error occured in the plugin, see errorString()
        NotStarted = 2,         // SpeechToText not set up or not listening
        NoMicrophone = 3,       // No microphone was found or the microphone is not accessible
        IncompatibleFormat = 4, // Incompatible microphone, must support PCM 16bit mono
        PermissionMissing = 5,  //
        Running = 6,            // The recognizer is set up and proceeds data
        Paused = 7              // The microphone input is paused
    };
    Q_ENUM(State);

    [[nodiscard]] inline State state() const { return m_state; }
    inline QString errorString() { return m_errorString; }

    [[nodiscard]] inline SpeechToTextPlugin *device() const { return m_plugin; }

    bool hasWord(const QString &word);

    void setWakeWord(const QString &word);
    QString wakeWord();

    QString language();

    void ask();

    explicit operator bool() const;

public Q_SLOTS:
    void setup();
    bool requestMicrophonePermission();

    void pause();
    void resume();

    void reset();

    void pluginStateChanged();

Q_SIGNALS:
    void stateChanged();
    void languageChanged();

    void answerReady(QString);

private Q_SLOTS:
    void onAnswerReady(const QString &);

    bool setUpModel();
    void setUpMic();

private:
    void setState(SpeechToText::State s);

    AUDIOINPUT *audio = nullptr;

    SpeechToTextPlugin *m_plugin = nullptr;
    QList<SpeechToTextPlugin *> m_plugins;

    State m_state = NotStarted;
    QString m_errorString;

    QString m_language;

    bool m_muted = false;
};

#endif // RECOGNIZER_H
