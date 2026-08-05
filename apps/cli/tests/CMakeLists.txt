add_executable(logscope_cli_tests
    cli_parser_test.cpp
    cli_parser_malformed_test.cpp
    investigation_command_test.cpp
    output_format_test.cpp
    report_output_test.cpp
    ../cli_analysis_config.cpp
    ../cli_parser.cpp
    ../investigation_command.cpp
    ../output_format.cpp
    ../report_config.cpp
    ../report_output.cpp
)

target_link_libraries(logscope_cli_tests
    PRIVATE
        logscope_gtest_support
        scope_analysis
        scope_configuration
        scope_foundation
        scope_investigation
        scope_reporting
        scope_workspace
        GTest::gtest_main
)

target_include_directories(logscope_cli_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_SOURCE_DIR}/tests
)

include(GoogleTest)

gtest_discover_tests(logscope_cli_tests)
