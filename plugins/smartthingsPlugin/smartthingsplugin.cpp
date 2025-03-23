#include "smartthingsplugin.h"
#include "devicemanager.h"
#include "languagemanager.h"
#include "networkhandler.h"
#include "smartthingssettingswidget.h"

#include <QJsonArray>

SmartThingsPlugin::SmartThingsPlugin(QObject *parent)
    : QObject(parent)
{}

SmartThingsPlugin::~SmartThingsPlugin()
{
    delete m_settingsWidget;
    delete m_languageManager;
    delete m_deviceManager;
    delete m_networkHandler;
}

void SmartThingsPlugin::setup()
{
    m_settingsWidget = new SmartThingsSettingsWidget();
    m_settingsWidget->setSettings(bridge->settings());
    bridge->registerSettingsWidget(m_settingsWidget);

    m_networkHandler = new NetworkHandler(m_settingsWidget, this);
    m_languageManager = new LanguageManager(this);
    m_deviceManager = new DeviceManager(m_networkHandler, this);

    m_languageManager->loadLanguageCommands();
}

bool SmartThingsPlugin::isValid(const QString &input)
{
    QJsonObject commands = m_languageManager->getCurrentCommands();
    QStringList deviceTypes = commands.keys();

    for (const QString &deviceType : deviceTypes) {
        QJsonObject deviceCommands = commands.value(deviceType).toObject();
        QList triggers = deviceCommands.value("triggers").toArray().toVariantList().toList();

        for (const QVariant &trigger : triggers) {
            if (input.contains(trigger.toString(), Qt::CaseInsensitive)) {
                return true;
            }
        }
    }
    return false;
}

void SmartThingsPlugin::run(const QString &input)
{
    auto devices = m_deviceManager->getDeviceList();
    QJsonObject commands = m_languageManager->getCurrentCommands();

    // Step 1: Check for specific device name
    for (const DeviceManager::Device &device : devices) {
        if (input.contains(device.deviceName, Qt::CaseInsensitive)) {
            if (device.categories.isEmpty()) {
                continue;
            }

            QString deviceType = device.categories.first();
            QJsonObject deviceCommands = commands.value(deviceType).toObject();
            QJsonArray onCommands = deviceCommands.value("on").toArray();
            QJsonArray offCommands = deviceCommands.value("off").toArray();

            bool turnOn = false;
            bool turnOff = false;

            for (const QJsonValue &cmd : onCommands) {
                if (input.contains(cmd.toString(), Qt::CaseInsensitive)) {
                    turnOn = true;
                    break;
                }
            }

            for (const QJsonValue &cmd : offCommands) {
                if (input.contains(cmd.toString(), Qt::CaseInsensitive)) {
                    turnOff = true;
                    break;
                }
            }

            if (turnOn && !turnOff) {
                m_deviceManager->controlDeviceById(device.deviceId, "on", deviceType);
            } else if (turnOff && !turnOn) {
                m_deviceManager->controlDeviceById(device.deviceId, "off", deviceType);
            } else {
                bridge->say(m_languageManager->getErrorMessage("ambiguous_command"));
            }
            return;
        }
    }

    // Step 2: Check for device type triggers
    QStringList deviceTypes = commands.keys();
    for (const QString &deviceType : deviceTypes) {
        if (deviceType == "errors" || deviceType == "responses") {
            continue;
        }

        QJsonObject deviceCommands = commands.value(deviceType).toObject();
        QJsonArray triggers = deviceCommands.value("triggers").toArray();
        bool typeMentioned = false;

        for (const QJsonValue &trigger : triggers) {
            if (input.contains(trigger.toString(), Qt::CaseInsensitive)) {
                typeMentioned = true;
                break;
            }
        }

        if (typeMentioned) {
            QJsonArray onCommands = deviceCommands.value("on").toArray();
            QJsonArray offCommands = deviceCommands.value("off").toArray();
            bool turnOn = false;
            bool turnOff = false;

            for (const QJsonValue &cmd : onCommands) {
                if (input.contains(cmd.toString(), Qt::CaseInsensitive)) {
                    turnOn = true;
                    break;
                }
            }

            for (const QJsonValue &cmd : offCommands) {
                if (input.contains(cmd.toString(), Qt::CaseInsensitive)) {
                    turnOff = true;
                    break;
                }
            }

            if (turnOn && !turnOff) {
                if (deviceType == "light") {
                    m_deviceManager->controlAllDevicesOfType(deviceType, "on");
                } else {
                    bridge->say(
                        m_languageManager->getErrorMessage("specify_device").arg(deviceType));
                }
            } else if (turnOff && !turnOn) {
                if (deviceType == "light") {
                    m_deviceManager->controlAllDevicesOfType(deviceType, "off");
                } else {
                    bridge->say(
                        m_languageManager->getErrorMessage("specify_device").arg(deviceType));
                }
            } else {
                bridge->say(m_languageManager->getErrorMessage("ambiguous_command"));
            }
            return;
        }
    }

    bridge->say(m_languageManager->getErrorMessage("command_not_recognized"));
}

void SmartThingsPlugin::setLanguage(const QString &language)
{
    m_languageManager->setLanguage(language);
}
