include(Dirs)

function(download_vosk_if_needed)
    # Set the URL for the ZIP file based on platform and architecture
    if (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ANDROID)
        if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
            set(VOSK_URL "https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-aarch64-0.3.45.zip")
        elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "armv7l")
            set(VOSK_URL "https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-armv7l-0.3.45.zip")
        elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "riscv64")
            set(VOSK_URL "https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-riscv64-0.3.45.zip")
        elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
            set(VOSK_URL "https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-x86_64-0.3.45.zip")
        elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86")
            set(VOSK_URL "https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-linux-x86-0.3.45.zip")
        endif()
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
        set(VOSK_URL "https://github.com/alphacep/vosk-api/releases/download/v0.3.42/vosk-osx-0.3.42.zip")
    elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        if (${CMAKE_SIZEOF_VOID_P} MATCHES "4")
            set(VOSK_URL "https://github.com/alphacep/vosk-api/releases/download/v0.3.42/vosk-win32-0.3.42.zip")
        elseif (${CMAKE_SIZEOF_VOID_P} MATCHES "8")
            set(VOSK_URL "https://github.com/alphacep/vosk-api/releases/download/v0.3.45/vosk-win64-0.3.45.zip")
        endif()
    endif()

    message(STATUS "Downloading vosk from ${VOSK_URL}")

    # Download and extract the ZIP file
    file(DOWNLOAD "${VOSK_URL}" "${VOSK_ZIP}" SHOW_PROGRESS STATUS VOSK_DOWNLOAD_STATUS)
    list(GET VOSK_DOWNLOAD_STATUS 0 VOSK_DOWNLOAD_ERROR)
    if(VOSK_DOWNLOAD_ERROR)
        message(WARNING "Failed to download vosk: ${VOSK_DOWNLOAD_ERROR}")
    endif()

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/tempDir")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf "${VOSK_ZIP}" WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/tempDir")

    # Move the extracted contents to another directory
    file(MAKE_DIRECTORY ${VOSK_DIR})

    file(GLOB VOSK_FILES "${CMAKE_BINARY_DIR}/tempDir/*/*")
    foreach(VOSK_FILE ${VOSK_FILES})
        file(COPY ${VOSK_FILE} DESTINATION ${VOSK_DIR})
    endforeach()
endfunction()
