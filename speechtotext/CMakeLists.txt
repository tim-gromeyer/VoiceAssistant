include_directories(${PROJECT_SOURCE_DIR}/speechtotext)

set(TTS_PLUGINS)

add_subdirectory(vosk-api)

if (NOT TTS_PLUGINS)
    message(FATAL_ERROR "No speech to text plugin found!")
endif()
