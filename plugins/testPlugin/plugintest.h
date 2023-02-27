#pragma once

#include "../base.h"

#include <QMainWindow>

class PluginTest : public QWidget, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_iid FILE "metadata.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit PluginTest(QObject *parent = nullptr)
        : QWidget(qobject_cast<QMainWindow *>(parent)){};

    bool isValid(const QString &) override;
    void run(const QString &) override;

private:
    using PluginInterface::bridge;
};
