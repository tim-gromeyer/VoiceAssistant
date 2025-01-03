cmake_minimum_required(VERSION 3.14)

project(mediaControl LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(Dirs)
include(Plugins)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt6 QUIET COMPONENTS DBus)

add_plugin(mediaControl CLASS_NAME PluginTest)
target_sources(mediaControl PUBLIC ../base.h ../bridge.h mediaControl.cpp mediaControl.h)
target_link_libraries(mediaControl PUBLIC Qt6::Widgets)

if(Qt6DBus_FOUND)
    target_compile_definitions(mediaControl PRIVATE HAS_DBUS)
    target_link_libraries(mediaControl PRIVATE Qt6::DBus)
endif()

add_custom_command(TARGET mediaControl POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:mediaControl> ${PLUGINS_DIR}
  COMMAND_EXPAND_LISTS
)

set(PLUGINS ${PLUGINS} mediaControl PARENT_SCOPE)
