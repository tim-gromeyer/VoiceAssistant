#pragma once

#include <QList>
#include <QObject>
#include <QString>

class NetworkHandler;

class DeviceManager : public QObject
{
    Q_OBJECT

public:
    struct Device
    {
        QString deviceId;
        QString deviceName;
        QStringList capabilities;
        QStringList categories;
    };

    explicit DeviceManager(NetworkHandler *networkHandler, QObject *parent = nullptr);

    QList<Device> getDeviceList();
    QString getDeviceId(const QString &deviceType);
    void controlDevice(const QString &deviceType, const QString &action);
    void controlDeviceById(const QString &deviceId,
                           const QString &action,
                           const QString &deviceType);
    void controlAllDevicesOfType(const QString &deviceType, const QString &action);

private:
    QList<Device> m_devices;
    NetworkHandler *m_networkHandler;
};
