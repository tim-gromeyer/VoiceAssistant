#pragma once

#include <QIODevice>

class SpeechToTextPlugin : public QIODevice
{
    Q_OBJECT
    Q_PROPERTY(QString wakeWord READ wakeWord WRITE setWakeWord NOTIFY wakeWordChanged)
    Q_PROPERTY(bool asking READ isAsking WRITE setAsking NOTIFY askingChanged)

public:
    explicit SpeechToTextPlugin(QObject *parent = nullptr){};

    // Reimplementation of the writeData method from QIODevice, which takes
    // the incoming audio data and size as arguments and processes them
    /// qint64 writeData(const char *data, qint64 size) override;

    // Ignore this
    inline qint64 readData(char *data, qint64 size) override { return size; };

    virtual QString wakeWord() = 0;
    virtual void setWakeWord(const QString &) = 0;

    // Asking mode. We don't emit the wakeWordDetected signal.
    virtual bool isAsking() = 0;
    virtual void setAsking(bool) = 0;

Q_SIGNALS:
    // Signal emitted when the wake word is detected in the audio input
    void wakeWordDetected();

    // Emit this signal when the wake word changes!
    void wakeWordChanged();
    void askingChanged(); // And this when the asking mode changes

    // Signal emitted when the recognizer has finished listening for audio input
    void doneListening();

    // Signal emitted when the recognizer has recognized some text in the audio input
    void textUpdated(const QString &text);

    void answerReady(QString);
};

#define SpeechToText_iid "voiceassistant.speechtotext/1.0"
Q_DECLARE_INTERFACE(SpeechToTextPlugin, SpeechToText_iid)
