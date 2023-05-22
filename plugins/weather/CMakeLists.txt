cmake_minimum_required(VERSION 3.14)

project(weatherPlugin LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(Dirs)
include(Plugins)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Positioning)

if (NOT Qt${QT_VERSION_MAJOR}Positioning_FOUND)
    message(WARNING "Qt Positioning not found. The weather plugin will not be build")
    return()
endif()

add_plugin(weatherPlugin CLASS_NAME WeatherPlugin)
target_sources(weatherPlugin PUBLIC
    ../base.h
    ../bridge.h
    ../utils.h
    weatherdata.cpp
    weatherdata.h
    weatherplugin.cpp
    weatherplugin.h
)
target_link_libraries(weatherPlugin PUBLIC
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Positioning
)

add_custom_command(TARGET weatherPlugin POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:weatherPlugin> ${PLUGINS_DIR}
  COMMAND_EXPAND_LISTS
)

set(PLUGINS ${PLUGINS} weatherPlugin PARENT_SCOPE)
