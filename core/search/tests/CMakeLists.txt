add_executable(scope_search_tests
    search_engine_test.cpp
    search_history_test.cpp
    search_query_parser_test.cpp
    search_query_test.cpp
    text_matcher_test.cpp
)

target_link_libraries(scope_search_tests
    PRIVATE
        scope_search
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_search_tests)
