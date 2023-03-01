#ifndef RECOGNIZER_H
#define RECOGNIZER_H

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

// Define the Listener class, which inherits from QIODevice
class Listener : public QIODevice
{
    Q_OBJECT

public:
    // Constructor for the Listener class, which takes a parent object
    // and calls the QIODevice constructor to open the device in read-write mode
    explicit Listener(QObject *parent);

    // Reimplementation of the writeData method from QIODevice, which takes
    // the incoming audio data and size as arguments and passes them to
    // the Vosk recognizer to convert the audio to text
    qint64 writeData(const char *data, qint64 size) override;

    inline qint64 readData(char *data, qint64 size) override { return size; };

Q_SIGNALS:
    // Signal emitted when the wake word is detected in the audio input
    void wakeWord();

    // Signal emitted when the recognizer has finished listening for audio input
    void doneListening();

    // Signal emitted when the recognizer has recognized some text in the audio input
    void textUpdated(const QString &text);

    void answerReady(QString);

private:
    // Private method that takes a JSON string containing the recognized text
    // and parses it to extract the text and emit the appropriate signals
    void parseText(const char *json);

    // Private method that takes a JSON string containing partial recognized text
    // and parses it to extract the text and emit the appropriate signals
    void parsePartial(const char *json);
};

class SpeechToText : public QObject
{
    Q_OBJECT

public:
    explicit SpeechToText(QObject *parent = nullptr);
    ~SpeechToText();

    enum State {
        NoModelFound = 0,       // No Vosk model was found for the system languages
        ModelsMissing = 1,      // The directory where the Vosk models are stored is empty
        ErrorWhileLoading = 2,  // Unknown error loading Vosk model or recognizer
        NotStarted = 3,         // SpeechToText not set up or not listening
        NoMicrophone = 4,       // No microphone was found or the microphone is not accessible
        IncompatibleFormat = 5, // Incompatible microphone, must support PCM 16bit mono
        Running = 6,            // The recognizer is set up and proceeds data
        Paused = 7              // The microphone input is paused
    };
    Q_ENUM(State);

    [[nodiscard]] inline State state() const { return m_state; }
    inline QString errorString() { return m_errorString; }

    [[nodiscard]] inline Listener *device() const { return m_device.get(); }

    static bool hasWord(const QString &word);

    static void setWakeWord(const QString &word);
    static QString wakeWord();

    inline QString language() { return m_language; };

    static void ask();

    void setup();

    explicit operator bool() const;

public Q_SLOTS:
    void pause();
    void resume();

    void reset();

Q_SIGNALS:
    void stateChanged();
    void languageChanged();

    void modelLoaded();

    void answerReady(QString);

private Q_SLOTS:
    void onAnswerReady(const QString &);

private:
    void setState(SpeechToText::State s);

    void setUpModel();
    void setUpMic();

    QScopedPointer<AUDIOINPUT> audio;
    QScopedPointer<Listener> m_device;

    State m_state = NotStarted;
    QString m_errorString;

    QString m_language;
};

#endif // RECOGNIZER_H
