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

    void setLanguage(const QString &language);

private slots:
    void handleNetworkResponse(QNetworkReply *reply);

private:
    struct Device
    {
        QString deviceId;
        QString deviceName; // label attribute
        QStringList capabilities;
        QStringList categories;
    };

    void loadLanguageCommands();
    QJsonObject getCurrentLanguageCommands() const;
    QString getAccessToken();
    QList<Device> getDeviceList();
    QString getDeviceId(const QString &deviceType);
    void controlDevice(const QString &deviceType, const QString &action);

    void controlDeviceById(const QString &deviceId,
                           const QString &action,
                           const QString &deviceType);
    void controlAllDevicesOfType(const QString &deviceType, const QString &action);

    SmartThingsSettingsWidget *settingsWidget;
    QNetworkAccessManager *manager;

    QJsonObject languageCommands;
    QString currentLanguage;

    QList<Device> m_devices;

    using PluginInterface::bridge;
};
