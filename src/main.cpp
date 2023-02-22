#include "mainwindow.h"
#include "utils.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

using namespace utils::literals;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(STR(":/logo/Icon.svg")));
    QApplication::setApplicationName(STR("VoiceAssistant"));

    QTranslator translator, qtTranslator;
    QString qtTranslationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (qtTranslationsPath.isEmpty())
        qtTranslationsPath = STR(":/qtTranslations/");

    // load translation for Qt
    if (qtTranslator.load(QLocale::system(), STR("qtbase"), STR("_"), qtTranslationsPath))
        QApplication::installTranslator(&qtTranslator);

    // try to load translation for current locale from resource file
    if (translator.load(QLocale::system(), STR("VoiceAssistant"), STR("_"), STR(":/translations")))
        QApplication::installTranslator(&translator);

    MainWindow w;
    w.show();
    return QApplication::exec();
}
