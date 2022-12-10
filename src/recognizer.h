#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <QIODevice>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioSource>
#define AUDIOINPUT QAudioSource
#else
#include <QAudioInput>
#define AUDIOINPUT QAudioInput
#endif

class Listener : public QIODevice
{
    Q_OBJECT
public:
    Listener(QObject *parent = nullptr);

    inline qint64 readData(char *data, qint64 size) override { return 0; };

    qint64 writeData(const char *data, qint64 size) override;

Q_SIGNALS:
    void textUpdated(const QString);
    void wakeWord();
    void doneListening();

private:
    void parseText(const char *json);
    void parsePartial(const char *json);

    bool waitWakeWort = true;
};

class Recognizer : public QObject
{
    Q_OBJECT
public:
    explicit Recognizer(QObject *parent = nullptr);
    ~Recognizer();

    enum State {
        Ok = 0,
        NoModelFound = 1, // No model for system languages found
        ModelsMissing = 2, // Model folder is empty
        ErrorWhileLoading = 3 // Unknown error
    };
    Q_ENUM(State);

    QString language;

    [[nodiscard]] inline State state() const { return m_state; }

    [[nodiscard]] inline Listener* device() const { return m_device.get(); }

    static bool hasWord(QString word);

    static void setWakeWord(const QString &word);
    static QString wakeWord();

    static void setModelDir(const QString &);
    static QString modelDir();

public Q_SLOTS:
    void setUpModel();
    void setUpMic();

Q_SIGNALS:
    void stateChanged();

private:
    inline void setState(Recognizer::State s) { m_state = s; Q_EMIT stateChanged(); };

    QScopedPointer<AUDIOINPUT> audio;
    QScopedPointer<Listener> m_device;

    State m_state = Ok;
};

#endif // RECOGNIZER_H
