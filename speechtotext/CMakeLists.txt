include_directories(${PROJECT_SOURCE_DIR}/speechtotext)

set(TTS_PLUGINS ${TTS_PLUGINS})

add_subdirectory(vosk-api)

if (NOT TTS_PLUGINS)
    message(WARNING "No speech to text plugin found!")
endif()

set(TTS_PLUGINS ${TTS_PLUGINS} PARENT_SCOPE)
