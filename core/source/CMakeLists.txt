# ==========================================================
# LogScope
# Core Source Manager
# ==========================================================

add_library(scope_source
    file_log_source.cpp
    source_dataset.cpp
    source_manager.cpp
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
