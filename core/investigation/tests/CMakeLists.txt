add_executable(scope_investigation_tests
    investigation_engine_test.cpp
    investigation_view_test.cpp
    line_count_filter_test.cpp
    log_level_filter_test.cpp
)

target_link_libraries(scope_investigation_tests
    PRIVATE
        scope_investigation
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_investigation_tests)
