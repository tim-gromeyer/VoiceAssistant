cmake_minimum_required(VERSION 3.14)

project(testPlugin LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(Dirs)
include(Plugins)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_plugin(testPlugin CLASS_NAME PluginTest)
target_sources(testPlugin PUBLIC ../base.h ../bridge.h plugintest.cpp plugintest.h)
target_link_libraries(testPlugin PUBLIC Qt6::Widgets)

add_custom_command(TARGET testPlugin POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:testPlugin> ${PLUGINS_DIR}
  COMMAND_EXPAND_LISTS
)

set(PLUGINS ${PLUGINS} testPlugin PARENT_SCOPE)
