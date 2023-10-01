#include "modeldownloader.h"
#include "global.h"
#include "utils.h"

#include <QClipboard>
#include <QDir>
#include <QElapsedTimer>
#include <QFutureWatcher>
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
#include <QThread>
#include <QVBoxLayout>
#include <QtConcurrentRun>

#include "elzip.hpp"

using namespace utils::strings::literals;

ModelDownloader::ModelDownloader(QWidget *parent)
    : QDialog{parent}
    , manager(new QNetworkAccessManager(this))
    , progress(new QProgressDialog(this))
    , downloadTime(new QElapsedTimer())
{
    setWindowTitle(tr("Model downloader"));

    progress->setWindowTitle(tr("Downloading..."));
    progress->setRange(0, 100);
    progress->close();

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    QDir().mkpath(dir::modelDir());
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
    searchBar->setPlaceholderText(tr("Search …"));
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
        connect(downloadButton,
                &QPushButton::clicked,
                this,
                &ModelDownloader::downloadModel,
                Qt::QueuedConnection);

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
    if (reply && reply->isRunning())
        return;

    // Get the sender QObject, which in this case should be a QPushButton
    senderButton = qobject_cast<QPushButton *>(sender());
    senderButton->setText(tr("Downloading"));
    senderButton->setEnabled(false);

    // Get the model info index
    currIndex = senderButton->property("index").toInt();

    // Get the ModelInfo
    const ModelInfo &info = modelInfos.at(currIndex);
    alreadyDownloaded = 0;
    endSize = info.size;

    QNetworkRequest request(info.url);

    // Get the file name
    QString fileName = info.name + STR(".zip");
    file = new QFile(dir::modelDir() + fileName, this);

    if (file->exists() && file->size() == info.size) {
        // File already exists and is complete,
        // skip the download and unzip
        QMetaObject::invokeMethod(this, &ModelDownloader::downloadFinished, Qt::QueuedConnection);
        return;
    }

    if (!file->open(QIODevice::Append)) {
        QMessageBox::critical(this,
                              tr("Could not open file"),
                              tr("Could not open %1 for writing:\n%2")
                                  .arg(file->fileName(), file->errorString()));
        return;
    }

    if (file->exists()) {
        alreadyDownloaded = file->size();
        request.setRawHeader("Range", "bytes=" + QByteArray::number(alreadyDownloaded) + "-");
    }

    // Create a QNetworkAccessManager object and use it to download the model
    reply = manager->get(request);

    // Connect the finished signal of the response to a lambda function
    // that will be called when the download is complete
    QObject::connect(reply, &QNetworkReply::finished, this, &ModelDownloader::downloadFinished);

    progress->setProperty("file", fileName);
    progress->setLabelText(tr("Downloading %1\n%2 from %3\n%4 - %5 remaining")
                               .arg(fileName,
                                    QString::number(alreadyDownloaded),
                                    QString::number(endSize),
                                    STR("0"),
                                    STR("0")));
    progress->setValue(0);
    connect(reply, &QNetworkReply::downloadProgress, this, &ModelDownloader::downloadProgress);
    connect(reply, &QIODevice::readyRead, this, &ModelDownloader::save);

    // Cancel if the user cancels
    connect(progress, &QProgressDialog::canceled, reply, &QNetworkReply::abort);
    downloadTime->start();

    progress->show();
}

void ModelDownloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    using namespace download;
    using namespace file;

    // Prevent overflow
    if (bytesTotal <= 0 || bytesTotal + alreadyDownloaded > (LLONG_MAX / 100))
        return;

    // Calculate download speed
    auto downloadSpeed = qint64(bytesReceived / double(downloadTime->elapsed() / 1000.0));

    // Calculate estimated time remaining
    qint64 bytesRemaining = bytesTotal - bytesReceived;
    qint64 secondsRemaining = bytesRemaining / downloadSpeed;

    // Update progress dialog
    progress->setValue(int((bytesReceived + alreadyDownloaded) * 100 / endSize));
    progress->setLabelText(tr("Downloading %1\n%2 from %3\n%4 - %5 remaining")
                               .arg(progress->property("file").toString(),
                                    makeSizeRedalbe(alreadyDownloaded + bytesReceived),
                                    makeSizeRedalbe(endSize),
                                    makeDownloadSpeedReadable(downloadSpeed),
                                    makeSecoundsReadable(secondsRemaining)));
}

void ModelDownloader::save()
{
    if (!file || !reply)
        return;

    file->write(reply->readAll());
}

void ModelDownloader::downloadFinished()
{
    if (!file)
        return;

    const ModelInfo &info = modelInfos.at(currIndex);
    bool error = false;

    // Hide the progress dialog
    progress->close();

    // Check for error
    if (reply && (reply->error() != QNetworkReply::NoError || !reply->isOpen())) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            QMessageBox::critical(this,
                                  tr("Download failed!"),
                                  tr("The download failed for following reason:\n%1")
                                      .arg(reply->errorString()));
        }

        if (senderButton) {
            senderButton->setText(tr("Download"));
            senderButton->setEnabled(true);
        }

        file->close();

        return;
    }

    // Read the downloaded data and write it to a file
    if (file->size() == info.size) {
    } else if (reply && file->isOpen()) {
        file->write(reply->readAll());
    } else {
        qWarning() << "File is not open! Check for previous error messages!";
        if (senderButton) {
            senderButton->setText(tr("Download"));
            senderButton->setEnabled(true);
        }
    }

    file->close();

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    qDebug() << "Unzipping" << file->fileName() << "to" << dir::modelDir();
    try {
        QFuture<void> future = QtConcurrent::run([this] {
            elz::extractZip(file->fileName().toStdString(), dir::modelDir().toStdString());
        });

        if (senderButton)
            senderButton->setText(tr("Unzipping"));

        QFutureWatcher<void> watcher;
        QEventLoop loop;
        connect(&watcher,
                &QFutureWatcherBase::finished,
                &loop,
                &QEventLoop::quit,
                Qt::QueuedConnection);
        watcher.setFuture(future);

        loop.exec();

        qDebug() << "Renaming folder ...";
        QDir dir(dir::modelDir());
        if (!dir.rename(info.name, info.lang)) {
            qCritical() << "Can't rename the folder, do it yourself!\nOriginal name:" << info.name
                        << "\nNew name:" << info.lang << "\n";
            error = true;
        }

    } catch (elz::zip_exception &e) {
        qCritical() << "Can't unzip voice model: " << e.what();
        if (senderButton) {
            senderButton->setText(tr("Failed. Try again"));
            senderButton->setEnabled(true);
        }
        error = true;
    }

    qDebug() << "Done";

    QGuiApplication::restoreOverrideCursor();

    if (senderButton)
        senderButton->setText(tr("Downloaded"));

    if (!error)
        Q_EMIT modelDownloaded();
}

ModelDownloader::~ModelDownloader()
{
    delete downloadTime;
}
