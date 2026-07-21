add_executable(scope_analysis_tests
    analysis_engine_test.cpp
    analysis_model_test.cpp
    format_detector_test.cpp
    json_lines_parser_test.cpp
    json_lines_summary_test.cpp
    log_line_classifier_test.cpp
    plain_text_field_extractor_test.cpp
    field_summary_test.cpp
    line_index_test.cpp
)

target_link_libraries(scope_analysis_tests
    PRIVATE
        scope_analysis
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_analysis_tests)
