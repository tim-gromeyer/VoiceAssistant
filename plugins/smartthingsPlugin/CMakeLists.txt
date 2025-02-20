cmake_minimum_required(VERSION 3.14)

project(smartThingsPlugin LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(Dirs)
include(Plugins)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# Set plugin sources and headers
set(SOURCES
    smartthingsplugin.cpp
    languagemanager.cpp
    devicemanager.cpp
    networkhandler.cpp
    smartthingsplugin.cpp
    smartthingssettingswidget.cpp
)

set(HEADERS
    smartthingsplugin.h
    languagemanager.h
    devicemanager.h
    networkhandler.h
    smartthingsplugin.h
    smartthingssettingswidget.h
)

# Add your plugin
add_plugin(smartThingsPlugin CLASS_NAME SmartThingsPlugin)
target_sources(smartThingsPlugin
    PUBLIC
        ../base.h ../bridge.h
        ressources.qrc
        ${SOURCES}
        ${HEADERS}
)

target_link_libraries(smartThingsPlugin PUBLIC
    Qt6::Widgets
    Qt6::Network
)

if(Qt6DBus_FOUND)
    target_compile_definitions(smartThingsPlugin PRIVATE HAS_DBUS)
    target_link_libraries(smartThingsPlugin PRIVATE Qt6::DBus)
endif()

# Copy the plugin to the plugins directory after building
add_custom_command(TARGET smartThingsPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:smartThingsPlugin> ${PLUGINS_DIR}
    COMMAND_EXPAND_LISTS
)

# Add this plugin to the list of plugins
set(PLUGINS ${PLUGINS} smartThingsPlugin PARENT_SCOPE)
