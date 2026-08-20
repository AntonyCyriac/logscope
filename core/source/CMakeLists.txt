# ==========================================================
# LogScope
# Core Source Manager
# ==========================================================

add_library(scope_source
    attributed_ingest_source.cpp
    composite_log_source.cpp
    discovery_census.cpp
    file_log_source.cpp
    instance_grouper.cpp
    rotation_grouper.cpp
    source_discovery.cpp
    tailing_file_log_source.cpp
    source_dataset.cpp
    source_manager.cpp
    stdin_log_source.cpp
    text_sniff.cpp
)

target_link_libraries(scope_source
    PUBLIC
        scope_foundation
        scope_runtime
)

target_include_directories(scope_source
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
