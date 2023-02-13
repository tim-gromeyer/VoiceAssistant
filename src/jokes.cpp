#include "jokes.h"
#include "mainwindow.h"
#include "utils.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

#include <chrono>

using namespace std::chrono_literals;
using namespace utils::literals;

Jokes::Jokes(QObject *parent)
    : QObject(parent)
    , manager(new QNetworkAccessManager(this))
{}

void Jokes::setup()
{
    qDebug() << "Jokes: setup";
    static const QStringList jokeLangs(
        {STR("cs"), STR("de"), STR("en"), STR("es"), STR("fr"), STR("pt")});
    static const QStringList uiLangs = QLocale::system().uiLanguages();

    jokeLang = std::accumulate(uiLangs.begin(),
                               uiLangs.end(),
                               QString(QLatin1String("")),
                               [](const QString &a, const QString &b) {
                                   const QString lang = b.right(2);
                                   return jokeLangs.contains(lang) ? lang : a;
                               });

    if (jokeLang.isEmpty()) {
        qCritical() << tr("No jokes are available for your language.\nYour languages: %1\nJoke "
                          "languages: %2")
                           .arg(uiLangs.join(STR(", ")), jokeLangs.join(STR(", ")));
        return;
    }

    fetchJokes();
    qDebug() << "Jokes: Setup finished";
}

void Jokes::tellJoke()
{
    qDebug() << "Jokes: Telling a joke. Cached amount of jokes:" << jokes.size();
    // TODO: Slow down voice temporarily
    if (jokes.isEmpty()) {
        fetchJokes();
        if (jokes.isEmpty()) {
            MainWindow::say(tr("Sorry, something went wrong"));
            return;
        }
    }

    Joke joke = jokes.takeFirst();
    previousJokes.append(joke);

    qDebug() << joke.joke << joke.delivery;
    if (jokes.size() == 1)
        QTimer::singleShot(3s, this, &Jokes::fetchJokes);

    if (joke.isSingle) {
        MainWindow::say(joke.joke);
        return;
    }

    MainWindow::sayAndWait(joke.joke);

    QEventLoop loop(this);

    QTimer timer(this);
    timer.setInterval(700ms);
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start();
    loop.exec();

    MainWindow::say(joke.delivery);
}

Jokes::Joke parseJson(const QByteArray &json)
{
    Jokes::Joke joke;
    QJsonDocument doc = QJsonDocument::fromJson(json);
    QJsonObject obj = doc.object();

    if (obj[STR("error")].toBool(false)) {
        joke.error = true;
        return joke;
    }
    joke.isSingle = obj[STR("type")].toString() == L1("single");
    if (joke.isSingle) {
        joke.joke = obj[STR("joke")].toString();
    } else {
        joke.joke = obj[STR("setup")].toString();
        joke.delivery = obj[STR("delivery")].toString();
    }

    return joke;
}

void Jokes::fetchJokes()
{
    qDebug() << "Jokes: Fetching jokes";
    int tries = 0;

    while (jokes.size() <= 2 && tries <= 5) {
        ++tries;

        QUrl url(STR("https://v2.jokeapi.dev/joke/Any?safe-mode&lang=%1").arg(jokeLang));
        reply = manager->get(QNetworkRequest(url));

        QEventLoop loop(this);
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            qCritical() << tr("Could not fetch joke: %1").arg(reply->errorString());
            return;
        }

        QByteArray data = reply->readAll();
        Joke joke = parseJson(data);
        if (joke.error)
            continue;

        if (!previousJokes.contains(joke))
            jokes.append(joke);
    }

    qDebug() << "Jokes: Fetching jokes completed. Number of cached jokes:" << jokes.size();
}
