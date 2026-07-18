add_executable(logscope_cli_tests
    cli_parser_test.cpp
    cli_parser_malformed_test.cpp
    output_format_test.cpp
    report_output_test.cpp
    ../cli_parser.cpp
    ../output_format.cpp
    ../report_config.cpp
    ../report_output.cpp
)

target_link_libraries(logscope_cli_tests
    PRIVATE
        scope_analysis
        scope_configuration
        scope_foundation
        scope_reporting
        GTest::gtest_main
)

target_include_directories(logscope_cli_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

include(GoogleTest)

gtest_discover_tests(logscope_cli_tests)
