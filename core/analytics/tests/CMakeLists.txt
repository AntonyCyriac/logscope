add_executable(scope_analytics_tests
    analytics_engine_test.cpp
    correlation_analyzer_test.cpp
    error_clusterer_test.cpp
    frequency_analyzer_test.cpp
    message_signature_test.cpp
    timeline_analyzer_test.cpp
    trend_analyzer_test.cpp
)

target_link_libraries(scope_analytics_tests
    PRIVATE
        scope_analytics
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_analytics_tests)
