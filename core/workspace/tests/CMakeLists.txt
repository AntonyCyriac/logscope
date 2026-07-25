add_executable(scope_workspace_tests
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
