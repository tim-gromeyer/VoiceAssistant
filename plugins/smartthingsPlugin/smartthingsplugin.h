#pragma once

#include "base.h"
#include <QNetworkAccessManager>
#include <QString>

class SmartThingsSettingsWidget;

class SmartThingsPlugin : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_iid FILE "smartthingsplugin.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit SmartThingsPlugin(QObject *parent = nullptr);
    ~SmartThingsPlugin() override = default;

    void setup() override;
    bool isValid(const QString &input) override;
    void run(const QString &input) override;

private slots:
    void handleNetworkResponse(QNetworkReply *reply);

private:
    QJsonObject makeRequest(const QString &url);

    QNetworkAccessManager *manager;

    QString getAccessToken();
    QString getDeviceId(const QString &deviceType);
    void controlDevice(const QString &deviceType, const QString &action);

    SmartThingsSettingsWidget *settingsWidget;

    using PluginInterface::bridge;
};
