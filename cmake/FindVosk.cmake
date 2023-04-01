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
    set(Vosk_FOUND OFF)
    return()
endif()

set(Vosk_FOUND ON)

add_library(Vosk INTERFACE)
target_link_libraries(Vosk INTERFACE "${VOSK_LIBRARY}")
target_link_directories(Vosk INTERFACE "${VOSK_DIR}")
target_include_directories(Vosk INTERFACE "${VOSK_DIR}")

if (CMAKE_CROSSCOMPILING)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
endif()
