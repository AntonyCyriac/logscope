add_executable(scope_analysis_tests
    analysis_engine_test.cpp
    analysis_model_test.cpp
    format_detector_test.cpp
    log_line_classifier_test.cpp
)

target_link_libraries(scope_analysis_tests
    PRIVATE
        scope_analysis
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_analysis_tests)
