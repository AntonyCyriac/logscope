# ==========================================================
# LogScope
# Core AI Tests
# ==========================================================

add_executable(scope_ai_tests
    ai_config_test.cpp
    ai_investigation_assistant_test.cpp
    ai_provider_test.cpp
    noop_ai_provider_test.cpp
)

target_link_libraries(scope_ai_tests
    PRIVATE
        scope_ai
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(scope_ai_tests)
