add_executable(logscope_integration_tests
    pipeline_integration_test.cpp
)

target_link_libraries(logscope_integration_tests
    PRIVATE
        scope_analysis
        scope_configuration
        scope_extension
        scope_foundation
        scope_investigation
        scope_reporting
        scope_runtime
        scope_source
        scope_workspace
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(logscope_integration_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
