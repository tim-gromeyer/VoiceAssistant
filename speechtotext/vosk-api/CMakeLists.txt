find_package(Vosk)
include(Plugins)

if (NOT ${Vosk_FOUND})
    message(WARNING "Vosk not found! Download vosk from https://github.com/alphacep/vosk-api/releases
                     and copy the library and the 'vosk-api.h' file in ${PROJECT_SOURCE_DIR}")
    return()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core)

add_plugin(voskPlugin)
target_sources(voskPlugin PUBLIC ../speechtotextplugin.h voskplugin.cpp voskplugin.h)
target_link_libraries(voskPlugin Qt${QT_VERSION_MAJOR}::Core Vosk)

set(TTS_PLUGINS ${TTS_PLUGINS} voskPlugin PARENT_SCOPE)

add_custom_command(TARGET voskPlugin POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:voskPlugin> ${SPEECHTOTEXT_PLUGINS_DIR}
  COMMAND_EXPAND_LISTS
)

install(DIRECTORY "${CMAKE_BINARY_DIR}/vosk/"
        DESTINATION speechtotext
)
