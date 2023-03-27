cmake_minimum_required(VERSION 3.13)

cmake_policy(SET CMP0077 NEW)

set(CMAKE_AUTOMOC OFF)
set(BUILD_SHARED_LIBS OFF)

set(ELZIP_DECOMPRESS_ONLY ON)

add_subdirectory(11Zip)

if (EMSCRIPTEN)
    target_compile_options(elzip PUBLIC "-pthread")
endif()
