#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

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
    // FIXME: multiple definition of `PluginBridge::metaObject()`. It works in the tests because of -DNO_BRIDGE
    Q_IMPORT_PLUGIN(PluginTest);
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
    parser.setApplicationDescription(
        translator.translate("cmd",
                             "Resource-efficient voice assistant that is still in the early stages "
                             "of development but already functional."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(a);

    MainWindow w;
    w.show();
    return QApplication::exec();
}
