add_executable(logscope_regression_tests
    ai_regression_test.cpp
    plugin_regression_test.cpp
    storage_regression_test.cpp
)

target_link_libraries(logscope_regression_tests
    PRIVATE
        scope_ai
        scope_analysis
        scope_extension
        scope_investigation
        scope_plugin
        scope_source
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(logscope_regression_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES LABELS "regression"
)
