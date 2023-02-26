#pragma once

#include <QString>
#include <QtPlugin>

#ifndef NO_BRIDGE
#include "bridge.h"
#else
class PluginBridge;
#endif

class PluginInterface
{
public:
    virtual ~PluginInterface() = default;

    virtual bool isValid(const QString &) = 0;

    virtual void run(const QString &) = 0;

    inline void setBridge(PluginBridge *b)
    {
#ifndef NO_BRIDGE
        bridge = b;
#endif
    }
    PluginBridge *bridge = nullptr;
};

#define PLUGIN_iid "io.github.tim-gromeyer.voiceassistant.plugin/1.0"
Q_DECLARE_INTERFACE(PluginInterface, PLUGIN_iid);
