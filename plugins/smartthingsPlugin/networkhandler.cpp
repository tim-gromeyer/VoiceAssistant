#include "networkhandler.h"
#include "smartthingssettingswidget.h"

#include <QDebug>
#include <QNetworkReply>

NetworkHandler::NetworkHandler(SmartThingsSettingsWidget *settingsWidget, QObject *parent)
    : QObject(parent)
    , m_settingsWidget(settingsWidget)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager,
            &QNetworkAccessManager::finished,
            this,
            &NetworkHandler::handleNetworkResponse);
}

QNetworkReply *NetworkHandler::get(QNetworkRequest &request)
{
    auto accesToken = getAccessToken();
    request.setRawHeader("Authorization", "Bearer " + getAccessToken().toUtf8());
    return m_manager->get(request);
}

QNetworkReply *NetworkHandler::post(QNetworkRequest &request, const QByteArray &data)
{
    request.setRawHeader("Authorization", "Bearer " + getAccessToken().toUtf8());
    return m_manager->post(request, data);
}

QString NetworkHandler::getAccessToken() const
{
    return m_settingsWidget->getAuthorizationToken();
}

void NetworkHandler::handleNetworkResponse(QNetworkReply *reply)
{
    // Note: This requires access to language commands for responses
    // Consider passing LanguageManager or emitting raw response
    if (reply->error() == QNetworkReply::NoError) {
        qDebug() << "Network request successful";
    } else {
        qDebug() << "Network error:" << reply->errorString();
    }
    reply->deleteLater();
    emit networkResponseReceived(reply);
}
