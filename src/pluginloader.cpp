#include "pluginloader.h"

#include "speechtotext/speechtotextplugin.h"

template<class PluginInterface>
PluginLoader<PluginInterface>::PluginLoader(QObject *parent)
    : QObject(parent)
{}

template<class PluginInterface>
void PluginLoader<PluginInterface>::loadPlugins(const QString &pluginDirPath)
{
    // First load static plugins
    loadStaticPlugins();

    // Then load dynamic plugins
    QDir pluginsDir(pluginDirPath);
    const auto entryList = pluginsDir.entryList(QDir::Files);

    for (const QString &fileName : entryList) {
        if (!QLibrary::isLibrary(fileName)) {
            continue;
        }

        QString filePath = pluginsDir.absoluteFilePath(fileName);
        loadDynamicPlugin(filePath);
    }
}

template<class PluginInterface>
void PluginLoader<PluginInterface>::loadStaticPlugins()
{
    const auto staticInstances = QPluginLoader::staticInstances();
    for (QObject *pluginObject : staticInstances) {
        pluginObject->setParent(this);
        auto *plugin = qobject_cast<PluginInterface *>(pluginObject);
        if (plugin) {
            m_plugins.append(plugin);
        }
    }
}

template<class PluginInterface>
bool PluginLoader<PluginInterface>::loadDynamicPlugin(const QString &filePath)
{
    QPluginLoader loader(filePath, this);

    if (!loader.load()) {
        qWarning() << "Failed to load plugin" << filePath
                   << "due to following reason:" << loader.errorString()
                   << "\nUse `qtplugininfo` for a more advanced error message!";
        return false;
    }

    QObject *pluginObject = loader.instance();
    if (!pluginObject) {
        return false;
    }

    pluginObject->setParent(this);
    auto *plugin = qobject_cast<PluginInterface *>(pluginObject);

    if (!plugin) {
        return false;
    }

    qInfo() << "Loaded plugin:" << plugin->pluginName();
    m_plugins.append(plugin);
    return true;
}

template<class PluginInterface>
PluginInterface *PluginLoader<PluginInterface>::loadPlugin(const QString &pluginName,
                                                           const QString &pluginDirPath)
{
    loadPlugins(pluginDirPath);

    for (auto *plugin : m_plugins) {
        if (plugin->pluginName() == pluginName) {
            return plugin;
        }
    }

    return nullptr;
}

template class PluginLoader<SpeechToTextPlugin>;
