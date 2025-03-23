#include "devicemanager.h"
#include "networkhandler.h"

#include <QDebug>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

DeviceManager::DeviceManager(NetworkHandler *networkHandler, QObject *parent)
    : QObject(parent)
    , m_networkHandler(networkHandler)
{}

QList<DeviceManager::Device> DeviceManager::getDeviceList()
{
    QNetworkRequest request(QUrl("https://api.smartthings.com/v1/devices"));

    QNetworkReply *reply = m_networkHandler->get(request);
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

    if (devicesList.empty()) {
        qDebug() << "No devices found";
    }

    reply->deleteLater();
    m_devices = devicesList;
    return devicesList;
}

QString DeviceManager::getDeviceId(const QString &deviceType)
{
    for (const Device &device : m_devices) {
        if (device.categories.contains(deviceType, Qt::CaseInsensitive)) {
            return device.deviceId;
        }
    }

    qDebug() << "No device found for type:" << deviceType;
    return "";
}

void DeviceManager::controlDevice(const QString &deviceType, const QString &action)
{
    QString deviceId = getDeviceId(deviceType);
    if (!deviceId.isEmpty()) {
        controlDeviceById(deviceId, action, deviceType);
    }
}

void DeviceManager::controlDeviceById(const QString &deviceId,
                                      const QString &action,
                                      const QString &deviceType)
{
    QString url = QString("https://api.smartthings.com/v1/devices/%1/commands").arg(deviceId);
    QNetworkRequest request((QUrl(url)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // Note: This requires access to language commands, which could be passed via constructor
    QJsonObject command;
    command["command"] = action;
    command["component"] = "main";
    command["capability"] = "switch";

    QJsonArray commandsArray;
    commandsArray.append(command);

    QJsonObject json;
    json["commands"] = commandsArray;

    QNetworkReply *reply = m_networkHandler->post(request, QJsonDocument(json).toJson());
    // Response handling is managed by NetworkHandler
}

void DeviceManager::controlAllDevicesOfType(const QString &deviceType, const QString &action)
{
    qInfo() << "Controlling all devices of type" << deviceType << "and executing action" << action;

    QList<QString> deviceIds;
    for (const Device &device : m_devices) {
        if (device.categories.contains(deviceType, Qt::CaseInsensitive)) {
            deviceIds.append(device.deviceId);
        }
    }

    for (const QString &deviceId : deviceIds) {
        controlDeviceById(deviceId, action, deviceType);
    }
}
