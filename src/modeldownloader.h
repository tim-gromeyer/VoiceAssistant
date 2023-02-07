#pragma once

#include <QDialog>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class QProgressDialog;
class QTableWidget;

struct ModelInfo
{
    QString lang;
    QString langText;
    QString name;
    QString sizeText;
    QUrl url;
    bool obsolete;
    int size;
};

class ModelDownloader : public QDialog
{
    Q_OBJECT
public:
    explicit ModelDownloader(QWidget *parent = nullptr);

Q_SIGNALS:
    void modelDownloaded();

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
