set(SPEECHTOTEXT_PLUGINS 0)
include_directories(${PROJECT_SOURCE_DIR}/speechtotext)

find_package(Vosk)

if (${Vosk_FOUND})
    add_subdirectory(vosk)
    MATH(EXPR SPEECHTOTEXT_PLUGINS "${SPEECHTOTEXT_PLUGINS}+1")
else()
    message(WARNING "Vosk not found! Download vosk from https://github.com/alphacep/vosk-api/releases\nand copy the library and the 'vosk-api.h' file in ${PROJECT_SOURCE_DIR}")
endif()

if (SPEECHTOTEXT_PLUGINS EQUAL 0)
    message(FATAL_ERROR "No speech to text plugin found!")
endif()
