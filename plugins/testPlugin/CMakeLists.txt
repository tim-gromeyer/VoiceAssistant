cmake_minimum_required(VERSION 3.14)

project(testPlugin LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(Dirs)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_library(testPlugin MODULE ../base.h ../bridge.h plugintest.cpp plugintest.h)
target_link_libraries(testPlugin Qt${QT_VERSION_MAJOR}::Widgets)

add_custom_command(TARGET testPlugin POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:testPlugin> ${PLUGINS_DIR}
  COMMAND_EXPAND_LISTS
)
