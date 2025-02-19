#pragma once

#include "../base.h"

#include <QMainWindow>

class PluginTest : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_iid FILE "metadata.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit PluginTest(QObject *parent = nullptr)
        : QObject(parent){};

    void setup() override{};
    bool isValid(const QString &) override;
    void run(const QString &) override;

private:
    using PluginInterface::bridge;
};
