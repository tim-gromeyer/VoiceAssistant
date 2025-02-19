find_package(Whisper)
include(Plugins)

if (NOT ${Whisper_FOUND})
    message(WARNING "Whisper not found!")
    return()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core)

add_plugin(whisperPlugin)
target_sources(whisperPlugin PUBLIC ../speechtotextplugin.h whisperplugin.h)
target_link_libraries(whisperPlugin Qt${QT_VERSION_MAJOR}::Core whisper)

set(TTS_PLUGINS ${TTS_PLUGINS} whisperPlugin PARENT_SCOPE)

add_custom_command(TARGET whisperPlugin POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:whisperPlugin> ${SPEECHTOTEXT_PLUGINS_DIR}
  COMMAND_EXPAND_LISTS
)

install(DIRECTORY "${CMAKE_BINARY_DIR}/whisper/"
        DESTINATION speechtotext
)
