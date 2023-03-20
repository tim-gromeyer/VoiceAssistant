add_executable(test_load_plugins test_load_plugins.cpp ../plugins/base.h)

target_link_libraries(test_load_plugins Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(test_load_plugins PRIVATE NO_BRIDGE PLUGIN="$<TARGET_FILE:testPlugin>")

enable_testing()
add_test(NAME test_load_plugins COMMAND test_load_plugins)
