# ==========================================================
# LogScope
# Core Storage Tests
# ==========================================================

add_executable(scope_storage_tests
    index_fingerprint_test.cpp
    index_reuse_test.cpp
    index_store_factory_test.cpp
    query_planner_test.cpp
    source_snapshot_test.cpp
    sqlite_connection_test.cpp
    sqlite_index_store_compression_test.cpp
    sqlite_index_store_fts_test.cpp
    sqlite_index_store_incremental_append_test.cpp
    sqlite_index_store_json_fields_test.cpp
    sqlite_index_store_query_cache_test.cpp
    sqlite_index_store_test.cpp
    storage_config_test.cpp
)

target_link_libraries(scope_storage_tests
    PRIVATE
        logscope_gtest_support
        scope_storage
        scope_analysis
        scope_runtime
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(scope_storage_tests)
