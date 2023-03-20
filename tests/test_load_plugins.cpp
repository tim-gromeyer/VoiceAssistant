#include "../plugins/base.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QPluginLoader loader;
    loader.setFileName(QStringLiteral(PLUGIN));
    if (!loader.load()) {
        QString errorMessage = QStringLiteral("Can not load the plugin %1: %2");
        errorMessage = errorMessage.arg(loader.fileName(), loader.errorString());

        std::cerr << errorMessage.toStdString() << std::endl;
        return 1;
    }

    QObject *object = loader.instance();
    if (!object) {
        std::cerr << "The plugin instance is nullptr! " << loader.errorString().toStdString()
                  << std::endl;
        return 1;
    }

    auto *interface = qobject_cast<PluginInterface *>(object);
    if (!interface) {
        std::cerr << "The plugin interface is nullptr" << std::endl;
        return 1;
    }

    if (!loader.unload()) {
        std::cerr << "Failed to unload plugin! " << loader.errorString().toStdString() << std::endl;
        return 1;
    }

    return 0;
}
