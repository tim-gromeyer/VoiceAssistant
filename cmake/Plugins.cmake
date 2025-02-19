get_target_property(__qtBuildQtType Qt${QT_VERSION_MAJOR}::Core TYPE)
if ("${__qtBuildQtType}" STREQUAL "STATIC_LIBRARY")
    set(QT_BUILD_STATIC ON)
else()
    set(QT_BUILD_STATIC OFF)
endif()

# Use the built-in qt_add_plugin function if Qt6 is available
function(add_plugin)
    qt_add_plugin(${ARGV})
    target_include_directories(${ARGV0} PRIVATE "${CMAKE_SOURCE_DIR}/plugins")
endfunction()
