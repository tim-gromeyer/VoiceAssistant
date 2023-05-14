# Plugins

## Creating a Plugin

This guide will walk you through the process of creating a plugin using the `PluginInterface`, `PluginBridge`, and `SettingsWidget` classes. These classes provide a foundation for building plugins that can communicate with a host application and have configurable settings.

Prerequisites:
- Basic knowledge of C++ and Qt framework.
- Development environment with Qt framework set up.

Step 1: Set up the Development Environment
- Ensure that you have a development environment with the Qt framework installed.
- Create a new project or open an existing project in your development environment.

Step 2: Define the Plugin Interface
- Include the necessary classes/headers: `PluginInterface` and `QtPlugin`.
- Create a new class that inherits from `PluginInterface`.
- Implement the pure virtual functions `PluginInterface::isValid` and `PluginInterface::run` according to your plugin's functionality.
- Optionally, you can define additional functions and member variables specific to your plugin.

Step 3: Include the Plugin Bridge (optional)
- Include the necessary classes: `PluginBridge`
- Use `PluginBridge::ask` to ask the user a question and get the answer
- Use `PluginBridge::say` or `PluginBridge::sayAndWait` to speech text
- Use the `PluginBridge::useWidget`-slot to set the content widget of the main window

Step 4: Create the Settings Widget (optional but suggested)
- Include the necessary headers/classes: `QWidget` and `QSettings`.
- Create a new class that inherits from `SettingsWidget`.
- Implement the pure virtual functions `SettingsWidget::apply` and `SettingsWidget::finish` to handle the plugin's settings.
- Customize the class by adding widgets and implementing their functionality.
- Use the provided functions to set the display name, display category, and category icon for the settings widget.

Step 5: Building the Plugin
- Build the plugin project to generate the plugin library file (DLL or SO file) based on your development environment.  
  You can use the `add_plugin` function defined in `cmake/Plugins.cmake` to automatically detect static build and the Qt version.
- Ensure that the plugin library is generated successfully.

Step 7: Testing and Deployment
- Test the plugin integration thoroughly within the host application to ensure proper functionality.
- Package the plugin and its dependencies for distribution or deployment.

Congratulations! You have successfully created a plugin using the `PluginInterface`, `PluginBridge`, and `SettingsWidget` classes. You can now extend the plugin's functionality, customize its settings widget, and integrate it with host applications that support the plugin system.

For more information and detailed examples, refer to the provided code documentation and the Qt framework documentation or see the example plugin ([testPlugin](testPlugin/)).
