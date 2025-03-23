#pragma once

#include <QDebug>
#include <QDir>
#include <QLibrary>
#include <QList>
#include <QObject>
#include <QPluginLoader>
#include <QString>

template<class PluginInterface>
class PluginLoader : public QObject
{
public:
    explicit PluginLoader(QObject *parent = nullptr);
    ~PluginLoader() = default;

    void loadPlugins(const QString &pluginDirPath);
    PluginInterface *loadPlugin(const QString &pluginName, const QString &pluginDirPath);
    QList<PluginInterface *> plugins() const { return m_plugins; }

private:
    void loadStaticPlugins();
    bool loadDynamicPlugin(const QString &filePath);

    QList<PluginInterface *> m_plugins;
};
