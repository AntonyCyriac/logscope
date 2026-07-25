# ==========================================================
# LogScope
# Core AI Tests
# ==========================================================

add_executable(scope_ai_tests
    ai_analytics_context_builder_test.cpp
    ai_anomaly_hint_formatter_test.cpp
    ai_config_test.cpp
    ai_context_builder_test.cpp
    ai_investigation_assistant_test.cpp
    ai_provider_test.cpp
    ai_summary_formatter_test.cpp
    nl_query_translator_test.cpp
    noop_ai_provider_test.cpp
)

target_link_libraries(scope_ai_tests
    PRIVATE
        scope_ai
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(scope_ai_tests)
