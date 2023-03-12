include(Dirs)

find_library(__voskFile
    NAMES vosk-api vosk libvosk
    PATHS ${VOSK_DIR}
    NO_DEFAULT_PATH
)

if (${__voskFile} STREQUAL "Vosk-NOTFOUND")
   set(Vosk_FOUND OFF)
   return()
endif()

get_filename_component(__vosk_path ${__voskFile} DIRECTORY)

set(Vosk_FOUND ON)

add_library(Vosk STATIC IMPORTED GLOBAL)
set_target_properties(Vosk PROPERTIES
    IMPORTED_LOCATION ${__voskFile}
    INTERFACE_INCLUDE_DIRECTORIES ${VOSK_DIR})
