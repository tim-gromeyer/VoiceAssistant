#include "modeldownloader.h"
#include "global.h"

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>


ModelDownloader::ModelDownloader(QWidget *parent)
    :QDialog(parent)
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    QNetworkAccessManager networkManager;

    connect(&networkManager, &QNetworkAccessManager::finished, this,
                     [&](QNetworkReply* reply){
        //this lambda is called when the reply is received
        //it can be a slot in your GUI window class
        //check for errors
        if(reply->error() != QNetworkReply::NoError){
            networkManager.clearAccessCache();
        } else {
            //parse the reply JSON and display result in the UI
            QJsonObject jsonObject = QJsonDocument::fromJson(reply->readAll()).object();
        }
        reply->deleteLater();
    });

    QNetworkRequest networkRequest(QUrl(STR("https://alphacephei.com/vosk/models/model-list.json")));
    networkManager.get(networkRequest);

}
