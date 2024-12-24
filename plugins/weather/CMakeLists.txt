cmake_minimum_required(VERSION 3.14)

project(weatherPlugin LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(Dirs)
include(Plugins)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

if(Qt6Positioning_FOUND)
    find_package(Qt6 REQUIRED COMPONENTS Positioning)
else()
    message(WARNING "Qt Positioning not found. The weather plugin will not be built.")
    return()
endif()

find_package(Qt6 REQUIRED COMPONENTS SvgWidgets)
set(SVG Qt6::SvgWidgets)

add_plugin(weatherPlugin CLASS_NAME WeatherPlugin)
target_sources(weatherPlugin PUBLIC
    ../base.h
    ../bridge.h
    ../utils.h
    weatherdata.cpp
    weatherdata.h
    weatherplugin.cpp
    weatherplugin.h
    position.cpp
    position.h
)
target_link_libraries(weatherPlugin PUBLIC
    Qt6::Widgets
    Qt6::Positioning
    Qt6::Network
    ${SVG}
)

add_custom_command(TARGET weatherPlugin POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:weatherPlugin> ${PLUGINS_DIR}
  COMMAND_EXPAND_LISTS
)

set(PLUGINS ${PLUGINS} weatherPlugin PARENT_SCOPE)
