#pragma once

#include <QString>
#include <QtPlugin>

#ifndef NO_BRIDGE
#include "bridge.h"
#else
class PluginBridge;
#endif

/**
 * @brief The PluginInterface class is an abstract interface for plugin functionality.
 *
 * PluginInterface provides an interface for defining and implementing plugin functionality.
 * Plugins that implement this interface can be loaded dynamically at runtime.
 */
class PluginInterface
{
public:
    virtual ~PluginInterface() = default;

    /**
     * @brief Set up everything in this function
     */
    virtual void setup() = 0;

    /**
     * @brief isValid checks if a plugin is valid for the given input.
     * @param input The input to check.
     * @return True if the plugin is valid for the given input, false otherwise.
     */
    virtual bool isValid(const QString &input) = 0;

    /**
     * @brief run executes the plugin functionality.
     * @param input The input to process.
     */
    virtual void run(const QString &input) = 0;

    /**
     * @brief setBridge sets the bridge for the plugin to communicate with the host application.
     * @param b The PluginBridge object.
     *
     * The bridge is used for communication between the plugin and the host application.
     * It allows the plugin to interact with the host application's functionality and services.
     */
    inline void setBridge(PluginBridge *b)
    {
#ifndef NO_BRIDGE
        bridge = b;
#endif
    }
    PluginBridge *bridge = nullptr;
};

/**
 * @brief The PLUGIN_iid macro defines the interface ID for the PluginInterface.
 *
 * This macro defines the unique interface ID for the PluginInterface.
 * It is used to identify and load the correct plugin at runtime.
 */
#define PLUGIN_iid "io.github.tim-gromeyer.voiceassistant.plugin/1.0"
Q_DECLARE_INTERFACE(PluginInterface, PLUGIN_iid);
