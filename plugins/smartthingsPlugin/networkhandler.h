#pragma once

#include <QNetworkAccessManager>
#include <QString>

class SmartThingsSettingsWidget;

class NetworkHandler : public QObject
{
    Q_OBJECT

public:
    explicit NetworkHandler(SmartThingsSettingsWidget *settingsWidget, QObject *parent = nullptr);

    QNetworkReply *get(QNetworkRequest &request);
    QNetworkReply *post(QNetworkRequest &request, const QByteArray &data);
    QString getAccessToken() const;

signals:
    void networkResponseReceived(QNetworkReply *reply);

private slots:
    void handleNetworkResponse(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager = nullptr;
    SmartThingsSettingsWidget *m_settingsWidget = nullptr;
};
