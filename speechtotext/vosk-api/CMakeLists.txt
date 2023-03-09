find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)

add_library(voskPlugin MODULE ../speechtotextplugin.h voskplugin.cpp voskplugin.h)
target_link_libraries(voskPlugin Qt${QT_VERSION_MAJOR}::Multimedia vosk)
target_include_directories(voskPlugin PUBLIC ${PROJECT_SOURCE_DIR}/vosk)

install(TARGETS voskPlugin LIBRARY DESTINATION lib)

list(APPEND ANDROID_EXTRA_LIBS ${CMAKE_SOURCE_DIR}/vosk/libvosk.so)
