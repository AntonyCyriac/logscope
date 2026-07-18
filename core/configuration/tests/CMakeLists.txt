add_executable(scope_configuration_tests
    configuration_manager_test.cpp
)

target_link_libraries(scope_configuration_tests
    PRIVATE
        scope_configuration
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_configuration_tests)
