# ==========================================================
# LogScope
# Core Source Manager
# ==========================================================

add_library(scope_source
    composite_log_source.cpp
    file_log_source.cpp
    source_dataset.cpp
    source_manager.cpp
    stdin_log_source.cpp
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
