find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS LinguistTools)

function(ADD_TRANSLATIONS res_file)
    if (NOT Qt${QT_VERSION_MAJOR}LinguistTools_FOUND)
        message(WARNING "Qt LinguistTools not found! Translations will not work!")
        return()
    endif()

    set(_rc_file ${CMAKE_CURRENT_BINARY_DIR}/app_translations.qrc)

    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        qt_add_translation(_QM_FILES ${ARGN})
    else()
        qt5_add_translation(_QM_FILES ${ARGN})
    endif()

    file(WRITE ${_rc_file} "<RCC>\n\t<qresource prefix=\"/translations\">\n")
    foreach(_lang ${_QM_FILES})
        get_filename_component(_filename ${_lang} NAME)
        file(APPEND ${_rc_file} "\t\t<file>${_filename}</file>\n")
    endforeach()
    file(APPEND ${_rc_file} "\t</qresource>\n</RCC>\n")

    set(${res_file} ${_rc_file} PARENT_SCOPE)
endfunction()
