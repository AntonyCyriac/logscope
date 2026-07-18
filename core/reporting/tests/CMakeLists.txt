add_executable(scope_reporting_tests
    report_formatter_test.cpp
    report_generator_test.cpp
    report_section_test.cpp
    report_test.cpp
)

target_link_libraries(scope_reporting_tests
    PRIVATE
        scope_reporting
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_reporting_tests)
