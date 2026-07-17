add_executable(scope_foundation_tests
    status_test.cpp
)

target_link_libraries(scope_foundation_tests
    PRIVATE
        scope_foundation
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_foundation_tests)
