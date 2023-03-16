if (TARGET Vosk)
    return()
endif()

include(Dirs)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)

isFileEmpty(${VOSK_ZIP} __voskZipIsNotEmpty)

if(${__voskZipIsNotEmpty})
    include(DownloadVosk)
    download_vosk_if_needed()
endif()

find_library(__voskFile
    NAMES vosk-api vosk libvosk
    PATHS ${VOSK_DIR}
    NO_DEFAULT_PATH
)

if (${__voskFile} STREQUAL "__voskFile-NOTFOUND")
    message(FATAL_ERROR "Vosk library file not found. Download it from https://github.com/alphacep/vosk-api/releases/tag/v0.3.42 and unpack it to ${VOSK_DIR}")
   set(Vosk_FOUND OFF)
   return()
endif()

get_filename_component(__vosk_path ${__voskFile} DIRECTORY)

set(Vosk_FOUND ON)

add_library(Vosk STATIC IMPORTED GLOBAL)
set_target_properties(Vosk PROPERTIES
    IMPORTED_LOCATION ${__voskFile}
    INTERFACE_INCLUDE_DIRECTORIES ${VOSK_DIR})
