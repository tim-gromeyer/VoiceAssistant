find_package(Vosk QUIET)

if (NOT ${Vosk_FOUND})
    message(WARNING "Vosk not found! Download vosk from https://github.com/alphacep/vosk-api/releases
                     and copy the library and the 'vosk-api.h' file in ${PROJECT_SOURCE_DIR}")
    return()
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(voskPlugin MODULE ../speechtotextplugin.h voskplugin.cpp voskplugin.h)
target_link_libraries(voskPlugin Qt${QT_VERSION_MAJOR}::Core Vosk)

set(TTS_PLUGINS ${TTS_PLUGINS} voskPlugin PARENT_SCOPE)
list(APPEND ANDROID_EXTRA_LIBS ${CMAKE_SOURCE_DIR}/vosk/libvosk.so)

add_custom_command(TARGET voskPlugin POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:voskPlugin> ${SPEECHTOTEXT_PLUGINS_DIR}
  COMMAND_EXPAND_LISTS
)
