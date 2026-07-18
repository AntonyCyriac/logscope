add_executable(scope_foundation_tests
    date_test.cpp
    datetime_test.cpp
    clock_test.cpp
    duration_test.cpp
    error_test.cpp
    filesystem_test.cpp
    path_test.cpp
    result_test.cpp
    status_test.cpp
    time_test.cpp
    timestamp_test.cpp
    uuid_test.cpp
    version_test.cpp
)

target_link_libraries(scope_foundation_tests
    PRIVATE
        scope_foundation
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(scope_foundation_tests)
