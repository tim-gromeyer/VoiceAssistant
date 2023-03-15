set(VOSK_DIR ${CMAKE_SOURCE_DIR}/vosk)

# plugins
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SPEECHTOTEXT_PLUGINS_DIR ${CMAKE_SOURCE_DIR}/speechtotext)
    set(PLUGINS_DIR ${CMAKE_SOURCE_DIR}/plugins)
else()
    set(SPEECHTOTEXT_PLUGINS_DIR ${CMAKE_BINARY_DIR}/speechtotext)
    set(PLUGINS_DIR ${CMAKE_BINARY_DIR}/plugins)
endif()
