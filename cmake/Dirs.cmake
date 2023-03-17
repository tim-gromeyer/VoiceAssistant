set(VOSK_DIR "${CMAKE_BINARY_DIR}/vosk")
set(VOSK_ZIP "${CMAKE_BINARY_DIR}/vosk.zip")

# plugins
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(SPEECHTOTEXT_PLUGINS_DIR ${CMAKE_SOURCE_DIR}/speechtotext)
    set(PLUGINS_DIR ${CMAKE_SOURCE_DIR}/plugins)
else()
    set(SPEECHTOTEXT_PLUGINS_DIR ${CMAKE_BINARY_DIR}/speechtotext)
    set(PLUGINS_DIR ${CMAKE_BINARY_DIR}/plugins)
endif()

function(isFileEmpty FILENAME RESULT_VARIABLE)
    if (NOT EXISTS ${FILENAME})
        set(${RESULT_VARIABLE} TRUE PARENT_SCOPE)
        return()
    endif()

    file(STRINGS "${FILENAME}" file_content)

    if(file_content STREQUAL "")
        set(${RESULT_VARIABLE} TRUE PARENT_SCOPE)
    else()
        set(${RESULT_VARIABLE} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Set installation directories
if (UNIX AND NOT APPLE AND NOT ANDROID)
    set(CPACK_SET_DESTDIR true)
    set(CPACK_INSTALL_PREFIX /opt/VoiceAssistant)
    set(CMAKE_INSTALL_PREFIX ${CPACK_INSTALL_PREFIX})
endif()
