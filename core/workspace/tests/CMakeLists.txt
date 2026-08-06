add_executable(scope_workspace_tests
    investigation_test.cpp
    investigation_timeline_test.cpp
    pstack_crash_analyzer_test.cpp
    session_store_test.cpp
    session_serializer_malformed_test.cpp
)

target_link_libraries(scope_workspace_tests
    PRIVATE
        logscope_gtest_support
        scope_workspace
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_workspace_tests)
