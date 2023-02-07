#include "modeldownloader.h"
#include "global.h"
#include "recognizer.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>

#include "elzip.hpp"

using namespace literals;

ModelDownloader::ModelDownloader(QWidget *parent)
    : QDialog{parent}
    , manager(new QNetworkAccessManager(this))
{
    setWindowTitle(tr("Model downloader"));
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QDir().mkpath(SpeechToText::modelDir());
    downloadInfo();
    setupUi();
    QGuiApplication::restoreOverrideCursor();
}

void ModelDownloader::downloadInfo()
{
    QFile f(QDir::tempPath() + STR("/VoiceAssistant-model-list.json"), this);
    QByteArray jsonData;
    if (!f.open(QIODevice::ReadOnly)) {
        reply = manager->get(
            QNetworkRequest(QUrl(STR("https://alphacephei.com/vosk/models/model-list.json"))));

        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            QGuiApplication::restoreOverrideCursor();
            QMessageBox::critical(this,
                                  tr("Error"),
                                  tr("Could not download model info file:\n%1")
                                      .arg(reply->errorString()));
            return;
        }

        jsonData = reply->readAll();

        QFile cache(f.fileName(), this);
        if (cache.open(QIODevice::WriteOnly)) {
            cache.write(jsonData);
            cache.close();
        }
    } else
        jsonData = f.readAll();
    qDebug() << "[debug] Cache file for model list:" << f.fileName();

    QJsonParseError error{};
    error.error = QJsonParseError::NoError;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(jsonData, &error);

    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this,
                             tr("Failed to load model information!"),
                             tr("Parsing error at %1:\n%2")
                                 .arg(QString::number(error.offset), error.errorString()));
        if (f.remove())
            downloadInfo();
        return;
    }

    // Process the content of the JSON file here, such as by getting and using the desired values
    QJsonArray jsonArray = jsonResponse.array();
    modelInfos.reserve(jsonArray.size());

    for (auto &&i : jsonArray) {
        QJsonObject jsonObject = i.toObject();
        ModelInfo modelInfo;
        modelInfo.lang = jsonObject[STR("lang")].toString();
        modelInfo.langText = jsonObject[STR("lang_text")].toString();
        modelInfo.name = jsonObject[STR("name")].toString();
        modelInfo.obsolete = jsonObject[STR("obsolete")].toBool();
        modelInfo.size = jsonObject[STR("size")].toInt();
        modelInfo.sizeText = jsonObject[STR("size_text")].toString();
        modelInfo.url = jsonObject[STR("url")].toString();

        // Add the struct to the list
        if (!modelInfo.obsolete)
            modelInfos.append(modelInfo);
    }
}

void ModelDownloader::setupUi()
{
    auto *layout = new QVBoxLayout(this);

    auto *searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText(tr("Search ..."));
    connect(searchBar, &QLineEdit::textChanged, this, &ModelDownloader::search);
    layout->addWidget(searchBar);

    table = new QTableWidget((int) modelInfos.size(), 4, this);
    table->setHorizontalHeaderLabels({tr("Language"), tr("Size"), tr("Name"), tr("Download")});

    int row = 0;

    // Create widgets to display the information and give the user the option to choose the desired model
    for (int i = 0; modelInfos.size() > i; ++i) {
        const ModelInfo &modelInfo = modelInfos.at(i);
        if (modelInfo.obsolete)
            // Skip obsolete models
            continue;

        QLocale l(modelInfo.lang);
        QString langText = l.nativeLanguageName();
        if (modelInfo.lang.size() > 2) {
            langText.append(L1(" ("));
            langText.append(l.nativeCountryName());
            langText.append(u')');
        }

        if (langText.isEmpty())
            langText = modelInfo.langText;

        auto *downloadButton = new QPushButton(tr("Download"), table);
        downloadButton->setProperty("index", i);
        connect(downloadButton, &QPushButton::clicked, this, &ModelDownloader::downloadModel);

        table->setItem(row, 0, new QTableWidgetItem(langText));
        table->setItem(row, 1, new QTableWidgetItem(modelInfo.sizeText));
        table->setItem(row, 2, new QTableWidgetItem(modelInfo.name));
        table->setCellWidget(row, 3, downloadButton);
        ++row;
    }
    table->sortByColumn(0, Qt::AscendingOrder);
    table->resizeColumnsToContents();
    layout->addWidget(table, 1);

    QMargins m(0, 0, 0, 0);
    setContentsMargins(m);
    layout->setContentsMargins(m);

    adjustSize();
    setLayout(layout);
}

void ModelDownloader::search(const QString &searchText)
{
    if (!table)
        return;

    // Search for the given text in the table widget
    QList<QTableWidgetItem *> matchingItems = table->findItems(searchText, Qt::MatchContains);

    // Create a list of rows that contain matching items
    QList<int> matchingRows;
    for (const QTableWidgetItem *item : matchingItems) {
        if (!item || item->column() == 4)
            continue;

        int row = item->row();
        if (!matchingRows.contains(row))
            matchingRows.append(row);
    }

    // Hide all rows that do not contain matching items
    for (int i = 0; i < table->rowCount(); ++i) {
        if (matchingRows.contains(i))
            table->setRowHidden(i, false);
        else
            table->setRowHidden(i, true);
    }
}

void ModelDownloader::downloadModel()
{
    // Get the sender QObject, which in this case should be a QPushButton
    senderButton = qobject_cast<QPushButton *>(sender());
    senderButton->setText(tr("Downloading"));
    senderButton->setEnabled(false);

    // Get the model info index
    currIndex = senderButton->property("index").toInt();

    // Get the ModelInfo
    const ModelInfo &info = modelInfos.at(currIndex);

    // Get the file name
    QString fileName = info.name + L1(".zip");

    // Create a QNetworkAccessManager object and use it to download the model
    reply = manager->get(QNetworkRequest(info.url));

    // Connect the finished signal of the response to a lambda function
    // that will be called when the download is complete
    QObject::connect(reply, &QNetworkReply::finished, this, &ModelDownloader::downloadFinished);

    // Create progress dialogue
    if (!progress)
        progress = new QProgressDialog(this);

    // Show a progress dialog to show the progress
    progress->setWindowTitle(tr("Downloading..."));
    progress->setLabelText(tr("Downloading %1").arg(fileName));
    progress->setValue(0);
    progress->setRange(0, info.size);
    connect(reply, &QNetworkReply::downloadProgress, this, [this](qint64 recived, qint64 total) {
        qDebug() << recived * 100 / total << "% downloaded";
        progress->setValue((int) recived);
    });

    // Cancel if the user cancels
    connect(progress, &QProgressDialog::canceled, reply, &QNetworkReply::abort);

    progress->exec();
}

void ModelDownloader::downloadFinished()
{
    const ModelInfo &info = modelInfos.at(currIndex);

    // Get the file name
    QString fileName = info.name + L1(".zip");

    // Hide the progress dialog
    if (progress)
        progress->close();

    // Check for error
    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::critical(this,
                              tr("Download failed!"),
                              tr("The download failed for following reason:\n%1")
                                  .arg(reply->errorString()));

        if (senderButton) {
            senderButton->setText(tr("Download"));
            senderButton->setEnabled(true);
        }

        return;
    }

    // Read the downloaded data and write it to a file
    QFile file(SpeechToText::modelDir() + fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reply->readAll());
        file.close();
    } else {
        QMessageBox::critical(this,
                              tr("Could not write file"),
                              tr("Can not write to file %1:\n%2")
                                  .arg(file.fileName(), file.errorString()));

        if (senderButton) {
            senderButton->setText(tr("Download"));
            senderButton->setEnabled(true);
        }
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QMessageBox dia(this);
    dia.setWindowTitle(tr("Unzipping file"));
    dia.setText(tr("Unzipping file ..."));
    dia.setIcon(QMessageBox::Information);
    dia.show();

    elz::extractZip(file.fileName().toStdString(), SpeechToText::modelDir().toStdString());
    QDir(SpeechToText::modelDir()).rename(info.name, info.lang);

    QGuiApplication::restoreOverrideCursor();
    dia.close();

    if (senderButton)
        senderButton->setText(tr("Downloaded"));

    Q_EMIT modelDownloaded();
}
