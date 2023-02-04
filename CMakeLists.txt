cmake_minimum_required(VERSION 3.13)

project(VoiceAssistant VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not specified. Release is used.")
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Disable debug messages in release mode and improve performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    string(REPLACE "-O2" "-O3" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Svg Multimedia Network TextToSpeech)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Svg Multimedia Network TextToSpeech)

set(TS_FILES translations/VoiceAssistant_de_DE.ts)
set(RC_FILES
    translations/translations.qrc
    ressources/ressources.qrc
)

include(cmake/Translation.cmake)

set(PROJECT_SOURCES
    ${RC_FILES}
    src/commanddialog.cpp
    src/commanddialog.h
    src/global.h
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/modeldownloader.cpp
    src/modeldownloader.h
    src/recognizer.cpp
    src/recognizer.h
    ui/mainwindow.ui
)

add_translations(TS ${TS_FILES})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(voiceassistant
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${TS}
    )
else()
    if(ANDROID)
        add_library(voiceassistant SHARED
            ${PROJECT_SOURCES}
            ${TS}
        )
    else()
        add_executable(voiceassistant
            ${PROJECT_SOURCES}
            ${TS}
        )
    endif()
endif()

IF(WIN32)
    SET(INTL_LDFLAGS -lintl)
ENDIF(WIN32)

add_subdirectory(3rdparty)

target_include_directories(voiceassistant PRIVATE src vosk)
target_link_directories(voiceassistant PRIVATE vosk)
target_link_libraries(voiceassistant PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::TextToSpeech
    ${INTL_LDFLAGS}
    vosk
    elzip
)

target_compile_definitions(voiceassistant PUBLIC
    APP_VERSION="${PROJECT_VERSION}"
    APP_DIR="${CMAKE_SOURCE_DIR}"
    QT_DISABLE_DEPRECATED_BEFORE=0x0${QT_VERSION_MAJOR}${QT_VERSION_MINOR}${QT_VERSION_PATCH}
)

set(ANDROID_EXTRA_LIBS
    ${CMAKE_SOURCE_DIR}/vosk/libvosk.so
)

set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android")

set_target_properties(voiceassistant PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.qtproject.voiceassistant
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    QT_ANDROID_EXTRA_LIBS "${ANDROID_EXTRA_LIBS}"
    QT_ANDROID_PACKAGE_SOURCE_DIR ${ANDROID_PACKAGE_SOURCE_DIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(voiceassistant)
endif()
