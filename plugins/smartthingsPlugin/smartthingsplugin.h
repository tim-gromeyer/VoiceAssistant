#pragma once

#include "base.h"
#include <QString>

class SmartThingsSettingsWidget;
class LanguageManager;
class DeviceManager;
class NetworkHandler;

class SmartThingsPlugin : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_iid FILE "smartthingsplugin.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit SmartThingsPlugin(QObject *parent = nullptr);
    ~SmartThingsPlugin() override;

    void setup() override;
    bool isValid(const QString &input) override;
    void run(const QString &input) override;
    void setLanguage(const QString &language);

private:
    SmartThingsSettingsWidget *m_settingsWidget = nullptr;
    LanguageManager *m_languageManager = nullptr;
    DeviceManager *m_deviceManager = nullptr;
    NetworkHandler *m_networkHandler = nullptr;

    using PluginInterface::bridge;
};
