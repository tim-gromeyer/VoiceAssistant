set(SPEECHTOTEXT_PLUGINS 0)
include_directories(${PROJECT_SOURCE_DIR}/speechtotext)

find_package(Vosk)

if (${Vosk_FOUND})
    add_subdirectory(vosk-api)
    list(APPEND TTS_PLUGINS voskPlugin)
else()
    message(WARNING "Vosk not found! Download vosk from https://github.com/alphacep/vosk-api/releases
                     and copy the library and the 'vosk-api.h' file in ${PROJECT_SOURCE_DIR}")
endif()

if (NOT TTS_PLUGINS)
    message(FATAL_ERROR "No speech to text plugin found!")
endif()
