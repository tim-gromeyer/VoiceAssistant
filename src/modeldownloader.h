#pragma once

#include <QDialog>
#include <QUrl>

class QTableWidget;
class QNetworkReply;
class QNetworkAccessManager;
class QProgressDialog;

struct ModelInfo
{
    QString name;
    QString lang;
    QString langText;
    QUrl url;
    int size;
    QString sizeText;
    bool obsolete;
};

class ModelDownloader : public QDialog
{
    Q_OBJECT
public:
    explicit ModelDownloader(QWidget *parent = nullptr);

private Q_SLOTS:
    void search(const QString &);
    void downloadFinished();

    void downloadInfo();
    void setupUi();

private:
    void downloadModel();

    int currIndex = 0;
    QPushButton *senderButton = nullptr;

    QList<ModelInfo> modelInfos;

    QTableWidget *table = nullptr;

    QNetworkAccessManager *manager = nullptr;
    QNetworkReply *reply = nullptr;

    QProgressDialog *progress = nullptr;
};
