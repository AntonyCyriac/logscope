add_executable(scope_compare_tests
    run_comparison_test.cpp
)

target_link_libraries(scope_compare_tests
    PRIVATE
        logscope_gtest_support
        scope_compare
        scope_analysis
        scope_source
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_compare_tests)
