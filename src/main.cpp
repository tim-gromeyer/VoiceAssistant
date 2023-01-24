#include "global.h"
#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

using namespace literals;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(STR(":/logo/Icon.svg")));

    QTranslator translator, qtTranslator;

    // load translation for Qt
    if (qtTranslator.load(QLocale::system(), STR("qtbase"), STR("_"), STR(":/qtTranslations/"))) {
        QApplication::installTranslator(&qtTranslator);
    }

    // try to load translation for current locale from resource file
    if (translator.load(QLocale::system(), STR("VoiceAssistant"), STR("_"), STR(":/translations"))) {
        QApplication::installTranslator(&translator);
    }

    MainWindow w;
    w.show();
    return QApplication::exec();
}
