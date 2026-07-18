add_executable(scope_runtime_tests
    configuration_test.cpp
    diagnostics_test.cpp
    plugin_registry_test.cpp
    service_registry_test.cpp
)

target_link_libraries(scope_runtime_tests
    PRIVATE
        scope_runtime
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_runtime_tests)
