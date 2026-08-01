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
    ai_json_util_test.cpp
    ai_provider_test.cpp
    ai_summary_formatter_test.cpp
    http_ai_mock_server.cpp
    http_ai_provider_test.cpp
    nl_query_translator_test.cpp
    noop_ai_provider_test.cpp
)

target_link_libraries(scope_ai_tests
    PRIVATE
        scope_ai
        httplib::httplib
        GTest::gtest_main
)

if(WIN32)
    target_link_libraries(scope_ai_tests PRIVATE ws2_32 crypt32)
endif()

include(GoogleTest)
gtest_discover_tests(scope_ai_tests
    DISCOVERY_TIMEOUT 60
)
