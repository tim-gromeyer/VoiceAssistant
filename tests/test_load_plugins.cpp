#include "../plugins/base.h"

#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>

#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

#if (QT_FEATURE_static == 1)
    Q_IMPORT_PLUGIN(PluginTest);
#endif

    // Test static plugins
    const auto staticPlugins = QPluginLoader::staticInstances();
    std::cout << "Amount of static plugins: " << staticPlugins.size() << '\n';

    for (auto *plugin : staticPlugins) {
        auto *interface = qobject_cast<PluginInterface *>(plugin);
        if (interface)
            return 0;
    }
    if (!staticPlugins.isEmpty()) {
        std::cerr << "Failed to cast at least one static plugin to PluginInterface" << std::endl;
        return 1;
    }

    // Test shared plugins
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
