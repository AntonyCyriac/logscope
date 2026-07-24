add_executable(scope_query_tests
    query_config_test.cpp
    query_evaluator_test.cpp
    query_parser_test.cpp
)

target_link_libraries(scope_query_tests
    PRIVATE
        scope_query
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_query_tests)
