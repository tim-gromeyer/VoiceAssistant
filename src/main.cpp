#include "mainwindow.h"

#include <QApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/logo/Icon.svg")));
    QApplication::setApplicationName(QStringLiteral("VoiceAssistant"));

    QTranslator translator, qtTranslator;
    QString qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (qtTranslationsPath.isEmpty())
        qtTranslationsPath = QStringLiteral(":/qtTranslations/");

    // load translation for Qt
    if (qtTranslator.load(QLocale::system(),
                          QStringLiteral("qtbase"),
                          QStringLiteral("_"),
                          qtTranslationsPath))
        QApplication::installTranslator(&qtTranslator);

    // try to load translation for current locale from resource file
    if (translator.load(QLocale::system(),
                        QStringLiteral("VoiceAssistant"),
                        QStringLiteral("_"),
                        QStringLiteral(":/translations")))
        QApplication::installTranslator(&translator);

    MainWindow w;
    w.show();
    return QApplication::exec();
}
