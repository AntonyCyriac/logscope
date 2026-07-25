# ==========================================================
# LogScope
# Core Plugin Tests
# ==========================================================

include(${CMAKE_SOURCE_DIR}/cmake/LogScopePluginSDK.cmake)

set(PLUGIN_TEST_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/plugins)

add_library(logscope_test_report_plugin SHARED test_report_plugin.cpp)
set_target_properties(logscope_test_report_plugin PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions(logscope_test_report_plugin PRIVATE LOGSCOPE_PLUGIN_EXPORT)
target_link_libraries(logscope_test_report_plugin PRIVATE logscope_plugin_sdk)

add_library(logscope_test_full_plugin SHARED test_full_plugin.cpp)
set_target_properties(logscope_test_full_plugin PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions(logscope_test_full_plugin PRIVATE LOGSCOPE_PLUGIN_EXPORT)
target_link_libraries(logscope_test_full_plugin PRIVATE logscope_plugin_sdk)

add_library(logscope_test_storage_plugin SHARED test_storage_plugin.cpp)
set_target_properties(logscope_test_storage_plugin PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions(logscope_test_storage_plugin PRIVATE LOGSCOPE_PLUGIN_EXPORT)
target_link_libraries(logscope_test_storage_plugin PRIVATE logscope_plugin_sdk)

set_target_properties(logscope_test_report_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_TEST_OUTPUT_DIR}/report
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_TEST_OUTPUT_DIR}/report
    ARCHIVE_OUTPUT_DIRECTORY ${PLUGIN_TEST_OUTPUT_DIR}/report
)

set_target_properties(logscope_test_full_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_TEST_OUTPUT_DIR}/full
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_TEST_OUTPUT_DIR}/full
    ARCHIVE_OUTPUT_DIRECTORY ${PLUGIN_TEST_OUTPUT_DIR}/full
)

set_target_properties(logscope_test_storage_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_TEST_OUTPUT_DIR}/storage
    LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_TEST_OUTPUT_DIR}/storage
    ARCHIVE_OUTPUT_DIRECTORY ${PLUGIN_TEST_OUTPUT_DIR}/storage
)

add_executable(scope_plugin_tests
    plugin_loader_test.cpp
    provider_integration_test.cpp
    shared_library_test.cpp
)

target_link_libraries(scope_plugin_tests
    PRIVATE
        scope_plugin
        scope_extension
        scope_analysis
        scope_investigation
        scope_reporting
        scope_search
        scope_source
        scope_storage
        GTest::gtest_main
)

target_compile_definitions(scope_plugin_tests PRIVATE
    PLUGIN_TEST_REPORT_DIR="${PLUGIN_TEST_OUTPUT_DIR}/report"
    PLUGIN_TEST_FULL_DIR="${PLUGIN_TEST_OUTPUT_DIR}/full"
    PLUGIN_TEST_STORAGE_DIR="${PLUGIN_TEST_OUTPUT_DIR}/storage"
    PLUGIN_TESTDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/testdata"
)

add_dependencies(scope_plugin_tests logscope_test_report_plugin logscope_test_full_plugin logscope_test_storage_plugin)

include(GoogleTest)
gtest_discover_tests(scope_plugin_tests)
