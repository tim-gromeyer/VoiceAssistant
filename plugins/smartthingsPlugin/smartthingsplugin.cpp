#include "smartthingsplugin.h"
#include "smartthingssettingswidget.h"
#include <QDebug>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

SmartThingsPlugin::SmartThingsPlugin(QObject *parent)
    : QObject(parent)
    , manager(nullptr)
    , settingsWidget(nullptr)
    , languageCommands()
    , currentLanguage(QLocale::system().name().left(2))
{}

void SmartThingsPlugin::setup()
{
    settingsWidget = new SmartThingsSettingsWidget();
    settingsWidget->setSettings(bridge->settings());
    bridge->registerSettingsWidget(settingsWidget);

    manager = new QNetworkAccessManager(this);

    // Load language commands from JSON file
    loadLanguageCommands();
}

void SmartThingsPlugin::loadLanguageCommands()
{
    QFile file(":/language_commands.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open language commands file";
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid language commands JSON format";
        return;
    }

    languageCommands = doc.object();
    qDebug() << "Loaded language commands for" << languageCommands.keys().size() << "languages";
}

void SmartThingsPlugin::setLanguage(const QString &language)
{
    if (languageCommands.contains(language)) {
        currentLanguage = language;
        qDebug() << "Language set to:" << language;
    } else {
        qWarning() << "Unsupported language:" << language << "- falling back to English";
        currentLanguage = "en";
    }
}

QJsonObject SmartThingsPlugin::getCurrentLanguageCommands() const
{
    return languageCommands.value(currentLanguage).toObject();
}

bool SmartThingsPlugin::isValid(const QString &input)
{
    QJsonObject commands = getCurrentLanguageCommands();

    // Check if input contains any device type keywords
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
    // Get the list of devices and current language commands
    m_devices = getDeviceList();
    QJsonObject commands = getCurrentLanguageCommands();

    // Step 1: Check if input contains a specific device name
    for (const Device &device : m_devices) {
        if (input.contains(device.deviceName, Qt::CaseInsensitive)) {
            // Assume the first category is the device type
            if (device.categories.isEmpty())
                continue;
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
                controlDeviceById(device.deviceId, "on", deviceType);
            } else if (turnOff && !turnOn) {
                controlDeviceById(device.deviceId, "off", deviceType);
            } else {
                bridge->say(
                    commands.value("errors").toObject().value("ambiguous_command").toString());
            }
            return;
        }
    }

    // Step 2: No specific device name found, check for device type triggers
    QStringList deviceTypes = commands.keys();
    for (const QString &deviceType : deviceTypes) {
        if (deviceType == "errors" || deviceType == "responses")
            continue;
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
                    controlAllDevicesOfType(deviceType, "on");
                } else {
                    bridge->say(commands.value("errors")
                                    .toObject()
                                    .value("specify_device")
                                    .toString()
                                    .arg(deviceType));
                }
            } else if (turnOff && !turnOn) {
                if (deviceType == "light") {
                    controlAllDevicesOfType(deviceType, "off");
                } else {
                    bridge->say(commands.value("errors")
                                    .toObject()
                                    .value("specify_device")
                                    .toString()
                                    .arg(deviceType));
                }
            } else {
                bridge->say(
                    commands.value("errors").toObject().value("ambiguous_command").toString());
            }
            return;
        }
    }

    // Step 3: No device name or type recognized
    bridge->say(commands.value("errors").toObject().value("command_not_recognized").toString());
}

void SmartThingsPlugin::handleNetworkResponse(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QJsonObject responseMessages = getCurrentLanguageCommands().value("responses").toObject();
        bridge->say(responseMessages.value("success").toString());
    } else {
        qDebug() << "Network error:" << reply->errorString();
        QJsonObject responseMessages = getCurrentLanguageCommands().value("responses").toObject();
        bridge->say(responseMessages.value("error").toString());
    }
    reply->deleteLater();
}

QString SmartThingsPlugin::getAccessToken()
{
    return settingsWidget->getAuthorizationToken();
}

QList<SmartThingsPlugin::Device> SmartThingsPlugin::getDeviceList()
{
    QNetworkRequest request(QUrl("https://api.smartthings.com/v1/devices"));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(getAccessToken()).toUtf8());

    QNetworkReply *reply = manager->get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QList<Device> devicesList;

    if (reply->error() == QNetworkReply::NoError) {
        QJsonObject devices = QJsonDocument::fromJson(reply->readAll()).object();
        QJsonArray items = devices["items"].toArray();
        for (const QJsonValue &item : items) {
            QJsonObject deviceObj = item.toObject();
            Device device;
            device.deviceId = deviceObj["deviceId"].toString();
            device.deviceName = deviceObj["label"].toString();

            QJsonArray components = deviceObj["components"].toArray();
            for (const QJsonValue &component : components) {
                QJsonArray categories = component.toObject()["categories"].toArray();
                for (const QJsonValue &category : categories) {
                    device.categories.append(category.toObject()["name"].toString());
                }

                QJsonArray capabilities = component.toObject()["capabilities"].toArray();
                for (const QJsonValue &capability : capabilities) {
                    device.capabilities.append(capability.toObject()["id"].toString());
                }
            }

            devicesList.append(device);
        }
    }

    if (devicesList.empty())
        qDebug() << "No devices found";
    reply->deleteLater();
    return devicesList;
}

QString SmartThingsPlugin::getDeviceId(const QString &deviceType)
{
    for (const Device &device : m_devices) {
        if (device.categories.contains(deviceType, Qt::CaseInsensitive)) {
            return device.deviceId;
        }
    }

    qDebug() << "No device found for type:" << deviceType;
    return "";
}

void SmartThingsPlugin::controlDevice(const QString &deviceType, const QString &action)
{
    QString deviceId = getDeviceId(deviceType);
    if (deviceId.isEmpty()) {
        QJsonObject errors = getCurrentLanguageCommands().value("errors").toObject();
        bridge->say(errors.value("device_not_found").toString().arg(deviceType));
        return;
    }

    QString url = QString("https://api.smartthings.com/v1/devices/%1/commands").arg(deviceId);
    QNetworkRequest request((QUrl(url)));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(getAccessToken()).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Get command configuration from the language file
    QJsonObject deviceConfig
        = getCurrentLanguageCommands().value(deviceType).toObject().value("config").toObject();

    QJsonObject command;
    command["command"] = action;
    command["component"] = deviceConfig.value("component").toString("main");
    command["capability"] = deviceConfig.value("capability").toString("switch");

    QJsonArray commands;
    commands.append(command);

    QJsonObject json;
    json["commands"] = commands;

    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    handleNetworkResponse(reply);
}

void SmartThingsPlugin::controlDeviceById(const QString &deviceId,
                                          const QString &action,
                                          const QString &deviceType)
{
    QJsonObject deviceConfig
        = getCurrentLanguageCommands().value(deviceType).toObject().value("config").toObject();
    QString url = QString("https://api.smartthings.com/v1/devices/%1/commands").arg(deviceId);
    QNetworkRequest request((QUrl(url)));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(getAccessToken()).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject command;
    command["command"] = action;
    command["component"] = deviceConfig.value("component").toString("main");
    command["capability"] = deviceConfig.value("capability").toString("switch");

    QJsonArray commands;
    commands.append(command);
    QJsonObject json;
    json["commands"] = commands;

    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleNetworkResponse(reply);
    });
}

void SmartThingsPlugin::controlAllDevicesOfType(const QString &deviceType, const QString &action)
{
    qInfo() << "Controlling all devices of type" << deviceType << "and executing action" << action;
    QList<QString> deviceIds;
    for (const Device &device : m_devices) {
        if (device.categories.contains(deviceType, Qt::CaseInsensitive)) {
            deviceIds.append(device.deviceId);
        }
    }
    if (deviceIds.isEmpty()) {
        QJsonObject errors = getCurrentLanguageCommands().value("errors").toObject();
        bridge->say(errors.value("device_not_found").toString().arg(deviceType));
        return;
    }
    for (const QString &deviceId : deviceIds) {
        controlDeviceById(deviceId, action, deviceType);
    }
}
