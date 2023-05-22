#pragma once

#include "../base.h"

class WeatherPlugin : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_iid FILE "metadata.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit WeatherPlugin(QObject *parent = nullptr)
        : QObject(parent){};

    bool isValid(const QString &) override;
    void run(const QString &) override;

private:
    using PluginInterface::bridge;
};
