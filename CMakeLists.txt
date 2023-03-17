cmake_minimum_required(VERSION 3.13)

project(VoiceAssistant VERSION 1.0.0 LANGUAGES CXX)
set(APP_VERSION "${CMAKE_PROJECT_VERSION}-alpha")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Build type not specified. Release is used.")
  set(CMAKE_BUILD_TYPE "Release")
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Disable debug messages in release mode and improve performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  string(REPLACE "-O2" "-O3" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
  string(REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  add_compile_definitions(QT_NO_DEBUG_OUTPUT)
  option(BUILD_TEST_PLUGIN "Build a test plugin" OFF)
else()
  option(BUILD_TEST_PLUGIN "Build a test plugin" ON)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Svg Multimedia Network TextToSpeech Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Svg Multimedia Network TextToSpeech Concurrent)

set(TS_FILES translations/VoiceAssistant_de_DE.ts)
set(RC_FILES translations/translations.qrc ressources/ressources.qrc)

include(Translation)

set(PROJECT_SOURCES
    ${RC_FILES}
    plugins/base.h
    plugins/bridge.h
    plugins/settingswidget.h
    speechtotext/speechtotextplugin.h
    src/commanddialog.cpp
    src/commanddialog.h
    src/global.h
    src/jokes.cpp
    src/jokes.h
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/modeldownloader.cpp
    src/modeldownloader.h
    src/recognizer.cpp
    src/recognizer.h
    src/settings.cpp
    src/settings.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/sliderwithtext.cpp
    src/sliderwithtext.h
    src/utils.cpp
    src/utils.h
    ui/mainwindow.ui
    ui/settingsdialog.ui
)

add_translations(TS ${TS_FILES})

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(voiceassistant MANUAL_FINALIZATION ${PROJECT_SOURCES} ${TS})
else()
  if(ANDROID)
    add_library(voiceassistant SHARED ${PROJECT_SOURCES} ${TS})
  else()
    add_executable(voiceassistant ${PROJECT_SOURCES} ${TS})
  endif()
endif()

if(WIN32)
  set(INTL_LDFLAGS -lintl)
endif(WIN32)

add_subdirectory(3rdparty)
add_subdirectory(speechtotext)
if(BUILD_TEST_PLUGIN)
  message(STATUS "Build test plugin")
  add_subdirectory(plugins/testPlugin)
endif()

target_include_directories(voiceassistant PRIVATE src)
target_link_libraries(voiceassistant PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Svg
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::TextToSpeech
    Qt${QT_VERSION_MAJOR}::Concurrent
    ${INTL_LDFLAGS}
    elzip)

target_compile_definitions(voiceassistant PUBLIC
    APP_DIR="${CMAKE_SOURCE_DIR}"
    APP_VERSION="${APP_VERSION}"
    QT_DISABLE_DEPRECATED_BEFORE=0x0${QT_VERSION_MAJOR}${QT_VERSION_MINOR}${QT_VERSION_PATCH}
    QT_USE_QSTRINGBUILDER
    QT${QT_VERSION_MAJOR}=1)

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

# Set installation directories
if (UNIX AND NOT APPLE AND NOT ANDROID)
    set(CPACK_SET_DESTDIR true)
    set(CPACK_INSTALL_PREFIX /opt/VoiceAssistant)
    set(CMAKE_INSTALL_PREFIX ${CPACK_INSTALL_PREFIX})
endif()

include(GNUInstallDirs)

# Install the main executable
install(TARGETS voiceassistant
    RUNTIME DESTINATION .
    BUNDLE DESTINATION . # Specify the bundle destination for MACOSX_BUNDLE
)

# Install TTS plugins
install(TARGETS ${TTS_PLUGINS}
        DESTINATION speechtotext
)

# Install other plugins
install(TARGETS ${PLUGINS}
        DESTINATION plugins
)

# Install the default commands
install(DIRECTORY commands
        DESTINATION .
)

# Create a symbolic link to the executable in /usr/local/bin
if (UNIX AND NOT APPLE)
    # install(CODE "execute_process(COMMAND ln -sf ${CMAKE_INSTALL_PREFIX}/voiceassistant /usr/local/bin/voiceassistant)")
endif()

include(Packaging)
