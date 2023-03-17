if (TARGET Vosk)
    return()
endif()

include(Dirs)

isFileEmpty(${VOSK_ZIP} __voskZipIsEmpty)

if(${__voskZipIsEmpty})
    include(DownloadVosk)
    download_vosk_if_needed()
endif()

if (CMAKE_CROSSCOMPILING)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
endif()

find_library(VOSK_LIBRARY
    NAMES vosk libvosk
    PATHS ${VOSK_DIR}
    NO_DEFAULT_PATH
)

if (VOSK_LIBRARY STREQUAL "VOSK_LIBRARY-NOTFOUND")
    message(FATAL_ERROR "Vosk library file not found. Download it from https://github.com/alphacep/vosk-api/releases/tag/v0.3.42 and unpack it to ${VOSK_DIR}")
    set(Vosk_FOUND OFF)
    return()
endif()

set(Vosk_FOUND ON)

add_library(Vosk STATIC IMPORTED GLOBAL)
set_target_properties(Vosk PROPERTIES
    IMPORTED_LOCATION ${VOSK_LIBRARY}
    IMPORTED_NO_SONAME true
    INTERFACE_INCLUDE_DIRECTORIES ${VOSK_DIR})

add_dependencies(Vosk ${VOSK_LIBRARY})

if (CMAKE_CROSSCOMPILING)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
endif()
