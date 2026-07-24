# ==========================================================
# LogScope
# Core Storage Layer
# ==========================================================

include(FetchContent)

FetchContent_Declare(
    sqlite_amalgamation
    URL https://www.sqlite.org/2024/sqlite-amalgamation-3450100.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(sqlite_amalgamation)

if(NOT TARGET sqlite3)
    add_library(sqlite3 STATIC ${sqlite_amalgamation_SOURCE_DIR}/sqlite3.c)
    target_include_directories(sqlite3 PUBLIC ${sqlite_amalgamation_SOURCE_DIR})
    target_compile_definitions(sqlite3 PUBLIC SQLITE_THREADSAFE=1)
endif()

add_library(scope_storage
    index_fingerprint.cpp
    index_reader.cpp
    index_store_factory.cpp
    query_planner.cpp
    schema_version.cpp
    sqlite_index_store.cpp
    storage_config.cpp
    workspace_paths.cpp
)

target_link_libraries(scope_storage
    PUBLIC
        scope_foundation
        scope_query
        scope_runtime
        sqlite3
    PRIVATE
        scope_analysis
)

target_include_directories(scope_storage
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
