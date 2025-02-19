#include "smartthingsplugin.h"
#include "smartthingssettingswidget.h"
#include <QDebug>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

SmartThingsPlugin::SmartThingsPlugin(QObject *parent)
    : QObject(parent)
{}

void SmartThingsPlugin::setup()
{
    settingsWidget = new SmartThingsSettingsWidget();
    settingsWidget->setSettings(bridge->settings());
    bridge->registerSettingsWidget(settingsWidget);

    manager = new QNetworkAccessManager(this);
}

bool SmartThingsPlugin::isValid(const QString &input)
{
    return input.contains("licht") || input.contains("outlet");
}

void SmartThingsPlugin::run(const QString &input)
{
    if (input.contains("licht") && input.contains("an")) {
        controlDevice("light", "on");
    } else if (input.contains("licht aus")) {
        controlDevice("light", "off");
    } else if (input.contains("turn on outlet")) {
        controlDevice("outlet", "on");
    } else if (input.contains("turn off outlet")) {
        controlDevice("outlet", "off");
    } else {
        bridge->say("Command not recognized.");
    }
}

void SmartThingsPlugin::handleNetworkResponse(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
        bridge->say("Done.");
    } else {
        qDebug() << "Network error:" << reply->errorString();
        bridge->say("There was an error executing that command.");
    }
    reply->deleteLater();
}

QString SmartThingsPlugin::getAccessToken()
{
    return settingsWidget->getAuthorizationToken();
}

QString SmartThingsPlugin::getDeviceId(const QString &deviceType)
{
    QNetworkRequest request(QUrl("https://api.smartthings.com/v1/devices"));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(getAccessToken()).toUtf8());

    QNetworkReply *reply = manager->get(request);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QJsonObject devices = QJsonDocument::fromJson(reply->readAll()).object();
        for (const QJsonValue &device : devices["items"].toArray()) {
            QJsonObject deviceObj = device.toObject();
            QJsonArray components = deviceObj["components"].toArray();
            for (const QJsonValue &component : components) {
                QJsonArray categories = component.toObject()["categories"].toArray();
                for (const QJsonValue &categorie : categories) {
                    if (categorie.toObject()["name"].toString().contains(deviceType,
                                                                         Qt::CaseInsensitive)) {
                        return deviceObj["deviceId"].toString();
                    }
                }
            }
        }
    }
    qDebug() << "No device found for type:" << deviceType;
    return "";
}

void SmartThingsPlugin::controlDevice(const QString &deviceType, const QString &action)
{
    QString deviceId = getDeviceId(deviceType);
    if (deviceId.isEmpty()) {
        bridge->say(QString("No %1 device found.").arg(deviceType));
        return;
    }

    QString url = QString("https://api.smartthings.com/v1/devices/%1/commands").arg(deviceId);
    QNetworkRequest request((QUrl(url)));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(getAccessToken()).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject command;
    if (deviceType == "light") {
        command["command"] = action == "on" ? "on" : "off";
        command["component"] = "main";
        command["capability"] = "switch";
    } else if (deviceType == "outlet") {
        command["command"] = action == "on" ? "on" : "off";
        command["component"] = "main";
        command["capability"] = "switch";
    }

    QJsonArray commands;
    commands.append(command);

    QJsonObject json;
    json["commands"] = commands;

    QNetworkReply *reply = manager->post(request, QJsonDocument(json).toJson());
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    handleNetworkResponse(reply);
}
