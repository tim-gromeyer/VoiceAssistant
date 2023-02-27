#pragma once

#include <QObject>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
class QTimer;
QT_END_NAMESPACE

class Jokes : public QObject
{
    Q_OBJECT

public:
    explicit Jokes(QObject *parent = nullptr);
    ~Jokes();

    struct Joke
    {
        bool error = false;

        bool isSingle = true;

        // Single joke
        QString joke;

        // For multi joke
        QString delivery;

        inline bool operator==(const Jokes::Joke &joke) const
        {
            return isSingle == joke.isSingle && this->joke == joke.joke
                   && delivery == joke.delivery;
        };
    };

public Q_SLOTS:
    void setup();

    void tellJoke();

    void fetchJokes(); // From the internet
    void loadJokes();  // From local file
    void saveJokes();  // Save to local file

private:
    QList<Joke> previousJokes;
    QList<Joke> jokes;

    QString jokeLang;

    QNetworkAccessManager *manager = nullptr;
    QNetworkReply *reply = nullptr;
};
