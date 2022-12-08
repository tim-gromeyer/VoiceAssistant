#ifndef RECOGNIZER_H
#define RECOGNIZER_H

#include <QAudioSource>
#include <QIODevice>

#include "vosk_api.h"


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

    QString language;

    Listener *device = nullptr;
public Q_SLOTS:
    void setUpModel();
    void setUpMic();

Q_SIGNALS:
    void stateChanged(Recognizer::State);

private:
    VoskModel *model = nullptr;
    VoskRecognizer *rec = nullptr;

    QAudioSource *audio = nullptr;
};

#endif // RECOGNIZER_H
