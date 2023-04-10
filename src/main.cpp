#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

void reformatJsonFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open JSON file: " << fileName;
        return;
    }

    QJsonParseError error{};

    QByteArray jsonData = file.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &error);

    file.close();

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse JSON file: " << error.errorString();
        return;
    }

    // write reformatted JSON data back to file
    file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
    file.write(jsonDoc.toJson(QJsonDocument::Indented));
    file.close();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName(QStringLiteral("VoiceAssistant"));
    QApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
    QApplication::setOrganizationName(QStringLiteral("VoiceAssistant"));
    QApplication::setOrganizationDomain(QStringLiteral("tim-gromeyer.github.io/VoiceAssistant"));
#ifdef Q_OS_DARWIN
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/logo/Icon.icns")));
#elif defined(Q_OS_WIN)
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/logo/Icon.ico")));
#else
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/logo/Icon.svg")));
#endif

#if (QT_FEATURE_static == 1)
//     FIXME: multiple definition of `PluginBridge::metaObject()`. It works in the tests because of -DNO_BRIDGE
//    Q_IMPORT_PLUGIN(PluginTest);
//    Q_IMPORT_PLUGIN(VoskPlugin);
#endif

    QTranslator translator, qtTranslator;
    QString qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (qtTranslationsPath.isEmpty())
        qtTranslationsPath = QStringLiteral(":/qtTranslations/");

    // load translation for Qt
    if (qtTranslator.load(QLocale::system(),
                          QStringLiteral("qt"),
                          QStringLiteral("_"),
                          qtTranslationsPath))
        QApplication::installTranslator(&qtTranslator);

    // try to load translation for current locale from resource file
    if (translator.load(QLocale::system(),
                        QStringLiteral("VoiceAssistant"),
                        QStringLiteral("_"),
                        QStringLiteral(":/translations")))
        QApplication::installTranslator(&translator);

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate(
        "cmd",
        "Resource-efficient voice assistant that is still in the early stages "
        "of development but already functional."));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption reformatOption(QStringLiteral("reformat"),
                                      QCoreApplication::translate("cmd", "Reformat JSON file"),
                                      QCoreApplication::translate("cmd", "file"));
    parser.addOption(reformatOption);

    parser.process(a);

    if (parser.isSet(reformatOption)) {
        QString fileName = parser.value(reformatOption);
        reformatJsonFile(fileName);
        return 0;
    }

    MainWindow w;
    w.show();
    return QApplication::exec();
}
