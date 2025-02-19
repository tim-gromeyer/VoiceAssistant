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

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Disable debug messages in release mode and improve performance
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    string(REPLACE "-O2" "-O3" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "-O2" "-O3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    add_compile_definitions(QT_NO_DEBUG_OUTPUT)
    option(BUILD_TEST_PLUGIN "Build a test plugin" OFF)
else()
    option(BUILD_TEST_PLUGIN "Build a test plugin" ON)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg Multimedia Network TextToSpeech Concurrent)

file(GLOB TS_FILES translations/VoiceAssistant_*.ts)
set(RC_FILES ressources/ressources.qrc)

include(Dirs)
include(Translation)
include(Plugins)

set(PROJECT_SOURCES
    ${RC_FILES}
    plugins/base.h
    plugins/bridge.h
    plugins/settingswidget.h
    plugins/utils.h
    speechtotext/speechtotextplugin.h
    src/commandwizard.cpp       src/commandwizard.h
    src/global.h
    src/jokes.cpp               src/jokes.h
    src/listwidget.h
    src/main.cpp
    src/mainwindow.cpp          src/mainwindow.h
    src/modeldownloader.cpp     src/modeldownloader.h
    src/pluginloader.cpp        src/pluginloader.h
    src/recognizer.cpp          src/recognizer.h
    src/settingsdialog.cpp      src/settingsdialog.h
    src/texttospeechsettings.cpp src/texttospeechsettings.h
    src/utils.cpp               src/utils.h
    src/commands.cpp            src/commands.h
    ui/mainwindow.ui
    ui/settingsdialog.ui
)

qt_add_executable(voiceassistant MANUAL_FINALIZATION ${PROJECT_SOURCES} ${TS})

add_subdirectory(3rdparty)
add_subdirectory(speechtotext)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
    set(BUILD_TEST_PLUGIN ON)
endif()

if(BUILD_TEST_PLUGIN)
  message(STATUS "Building a test plugin")
  add_subdirectory(plugins/testPlugin)
  add_subdirectory(plugins/weather)
endif()

add_subdirectory(plugins/mediaControl)
add_subdirectory(plugins/smartthingsPlugin)

target_include_directories(voiceassistant PRIVATE src)
target_link_libraries(voiceassistant PRIVATE
    Qt6::Widgets
    Qt6::Svg
    Qt6::Multimedia
    Qt6::Network
    Qt6::TextToSpeech
    Qt6::Concurrent
    elzip
)

target_compile_definitions(voiceassistant PUBLIC
    APP_DIR="${CMAKE_SOURCE_DIR}"
    APP_VERSION="${APP_VERSION}"
    QT_USE_QSTRINGBUILDER
)

set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android")

set_target_properties(voiceassistant PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.qtproject.voiceassistant
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    QT_ANDROID_EXTRA_LIBS "${CMAKE_BINARY_DIR}/plugins/libplugins__voskPlugin_arm64-v8a.so;${CMAKE_BINARY_DIR}/vosk/libvosk.so;/home/tim/qtprojegt/build-VoiceAssistant-Android_Qt_6_5_0_Clang_arm64_v8a-Debug/plugins/libatomic.so"
    QT_ANDROID_PACKAGE_SOURCE_DIR ${ANDROID_PACKAGE_SOURCE_DIR}
)

if (EMSCRIPTEN)
    target_link_options(voiceassistant PUBLIC "-sASYNCIFY -Os")
endif()

qt_finalize_executable(voiceassistant)

include(GNUInstallDirs)

# Install the main executable
install(TARGETS voiceassistant
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION . # Specify the bundle destination for MACOSX_BUNDLE
    LIBRARY DESTINATION . # For Android
)

if(QT_VERSION GREATER_EQUAL 6.3.0)
    if (QT_VERSION GREATER_EQUAL 6.5.0)
        qt_generate_deploy_app_script(
            TARGET voiceassistant
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
        )
    else()
        qt_generate_deploy_app_script(
            TARGET voiceassistant
            FILENAME_VARIABLE deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
        )
    endif()
    install(SCRIPT ${deploy_script})
endif()

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
