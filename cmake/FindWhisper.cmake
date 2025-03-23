# FindWhisper.cmake
# This module defines
#  WHISPER_FOUND - Whether whisper is found
#  WHISPER_INCLUDE_DIRS - Where to find whisper.h
#  WHISPER_LIBRARIES - The libraries to link against
#  WHISPER_TARGET - The imported target if available

# First check if the target already exists (e.g., from add_subdirectory)
if(TARGET whisper)
    set(WHISPER_FOUND TRUE)
    # Create an imported target if it doesn't exist
    if(NOT TARGET Whisper::Whisper)
        add_library(Whisper::Whisper ALIAS whisper)
    endif()
    return()
endif()

# Check if we have a submodule
set(WHISPER_SUBMODULE_DIR "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/whisper.cpp")
if(EXISTS "${WHISPER_SUBMODULE_DIR}/CMakeLists.txt")
    # Configure whisper build options
    set(WHISPER_STANDALONE OFF)
    set(WHISPER_BUILD_TESTS OFF)
    set(WHISPER_BUILD_EXAMPLES OFF)

    # Add the subdirectory
    add_subdirectory(${WHISPER_SUBMODULE_DIR} ${CMAKE_BINARY_DIR}/whisper)

    # Create our interface target
    add_library(Whisper::Whisper ALIAS whisper)

    set(WHISPER_FOUND TRUE)
    set(WHISPER_INCLUDE_DIRS ${WHISPER_SUBMODULE_DIR})
    set(WHISPER_LIBRARIES whisper)

    # Print status
    message(STATUS "Found whisper.cpp (submodule):")
    message(STATUS "  - Include dirs: ${WHISPER_INCLUDE_DIRS}")
    message(STATUS "  - Using target: whisper")

    return()
endif()

# If we get here, try to find system installation
include(FindPackageHandleStandardArgs)

# Search for whisper.h
find_path(WHISPER_INCLUDE_DIR
    NAMES whisper.h
    PATHS
        ${WHISPER_ROOT_DIR}
        ${WHISPER_ROOT_DIR}/include
        /usr/include
        /usr/local/include
    PATH_SUFFIXES whisper
)

# Search for the library
if(WIN32)
    set(WHISPER_NAMES whisper)
else()
    set(WHISPER_NAMES whisper libwhisper.so libwhisper.a)
endif()

find_library(WHISPER_LIBRARY
    NAMES ${WHISPER_NAMES}
    PATHS
        ${WHISPER_ROOT_DIR}
        ${WHISPER_ROOT_DIR}/lib
        /usr/lib
        /usr/local/lib
)

# Handle the QUIETLY and REQUIRED arguments and set WHISPER_FOUND
find_package_handle_standard_args(Whisper
    REQUIRED_VARS
        WHISPER_LIBRARY
        WHISPER_INCLUDE_DIR
    FAIL_MESSAGE "Could not find whisper.cpp - please install it, specify WHISPER_ROOT_DIR, or initialize the submodule"
)

if(WHISPER_FOUND)
    set(WHISPER_LIBRARIES ${WHISPER_LIBRARY})
    set(WHISPER_INCLUDE_DIRS ${WHISPER_INCLUDE_DIR})

    # Create imported target
    if(NOT TARGET Whisper::Whisper)
        add_library(Whisper::Whisper UNKNOWN IMPORTED)
        set_target_properties(Whisper::Whisper PROPERTIES
            IMPORTED_LOCATION "${WHISPER_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${WHISPER_INCLUDE_DIR}"
        )
    endif()

    # Print status
    message(STATUS "Found whisper.cpp (system):")
    message(STATUS "  - Include dirs: ${WHISPER_INCLUDE_DIRS}")
    message(STATUS "  - Library: ${WHISPER_LIBRARIES}")
else()
    message(WARNING "whisper.cpp not found - plugin will not be built")
    return()
endif()
