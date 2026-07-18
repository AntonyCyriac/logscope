add_executable(scope_source_tests
    file_log_source_test.cpp
    source_manager_test.cpp
    stdin_log_source_test.cpp
)

target_link_libraries(scope_source_tests
    PRIVATE
        scope_source
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_source_tests)
