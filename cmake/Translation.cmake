function(ADD_TRANSLATIONS res_file)
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
