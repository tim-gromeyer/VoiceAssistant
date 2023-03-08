find_library(Vosk
    NAMES vosk-api vosk libvosk
    HINTS ${PROJECT_SOURCE_DIR}
)

if (${Vosk} STREQUAL "Vosk-NOTFOUND")
   set(Vosk_FOUND OFF)
else()
    set(Vosk_FOUND ON)
endif()
