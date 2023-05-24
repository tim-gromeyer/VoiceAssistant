#include "position.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QtPositioning/QGeoCoordinate>
#include <QtPositioning/QGeoPositionInfo>
#include <QtPositioning/QGeoPositionInfoSource>

Position::Position(QObject *parent)
    : QObject(parent)
    , source(QGeoPositionInfoSource::createDefaultSource(this))
{
    connect(source, &QGeoPositionInfoSource::positionUpdated, this, &Position::positionUpdated);
}

void Position::positionUpdated(const QGeoPositionInfo &info)
{
    QGeoCoordinate coordinate = info.coordinate();
    m_latitude = coordinate.latitude();
    m_longitude = coordinate.longitude();
}

void Position::getUserPosition(double *latitude, double *longitude) const
{
    if (latitude == nullptr || longitude == nullptr)
        return;

    *latitude = m_latitude;
    *longitude = m_longitude;
}

void Position::getCityPosition(QStringView city, double *latitude, double *longitude)
{
    if (latitude == nullptr || longitude == nullptr)
        return;

    QNetworkAccessManager manager;
    QUrl url(QStringLiteral("https://geocoding-api.open-meteo.com/v1/search"));

    QUrlQuery query;
    query.addQueryItem(QStringLiteral("name"), city.toString());
    query.addQueryItem(QStringLiteral("count"), {u'1'});

    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    while (reply->isRunning())
        QCoreApplication::processEvents();

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << tr("Could not fetch joke: %1").arg(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (jsonDoc.isNull()) {
        qWarning() << "Failed to parse city information.";
        return;
    }

    // Extract the latitude and longitude
    QJsonObject jsonObject = jsonDoc.object();
    if (jsonObject.contains(QStringLiteral("error"))) {
        qWarning() << "Error:" << jsonObject[QStringLiteral("reason")].toString();
        return;
    }

    QJsonArray resultsArray = jsonObject[QStringLiteral("results")].toArray();
    if (resultsArray.isEmpty()) {
        qWarning() << "No results found for city:" << city;
        return;
    }

    QJsonObject resultObject = resultsArray.at(0).toObject();
    *latitude = resultObject[QStringLiteral("latitude")].toDouble();
    *longitude = resultObject[QStringLiteral("longitude")].toDouble();
}
