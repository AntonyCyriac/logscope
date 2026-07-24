# ==========================================================
# LogScope
# Core Storage Tests
# ==========================================================

add_executable(scope_storage_tests
    index_fingerprint_test.cpp
    index_store_factory_test.cpp
    query_planner_test.cpp
    sqlite_index_store_test.cpp
    storage_config_test.cpp
)

target_link_libraries(scope_storage_tests
    PRIVATE
        scope_storage
        scope_analysis
        scope_runtime
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(scope_storage_tests)
