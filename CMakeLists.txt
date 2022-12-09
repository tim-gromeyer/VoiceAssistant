cmake_minimum_required(VERSION 3.12)

project(VoiceAssistant VERSION 1.3.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Svg Concurrent Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Svg Concurrent Multimedia)

set(TS_FILES translations/VoiceAssistant_de_DE.ts)
set(RC_FILES
    translations/translations.qrc
    ressources/ressources.qrc
)

include(cmake/Translation.cmake)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        ui/mainwindow.ui
        src/recognizer.h
        src/recognizer.cpp
        src/global.h
        ${RC_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    add_translations(TS ${TS_FILES})

    qt_add_executable(voiceassistant
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        ${TS}
    )

else()
    add_translations(TS ${TS_FILES})

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

target_include_directories(voiceassistant PRIVATE src vosk)
target_link_directories(voiceassistant PRIVATE vosk)
target_link_libraries(voiceassistant PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Multimedia
    ${INTL_LDFLAGS}
    vosk
)

target_compile_definitions(voiceassistant PUBLIC
    APP_VERSION="${PROJECT_VERSION}"
    APP_DIR="${CMAKE_SOURCE_DIR}"
)

set_target_properties(voiceassistant PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.qtproject.voiceassistant
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(voiceassistant)
endif()
