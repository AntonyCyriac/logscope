add_executable(scope_extension_tests
    extension_manager_test.cpp
)

target_link_libraries(scope_extension_tests
    PRIVATE
        scope_extension
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_extension_tests)
