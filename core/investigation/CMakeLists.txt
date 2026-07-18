# ==========================================================
# LogScope
# Core Investigation Engine
# ==========================================================

add_library(scope_investigation
    investigation_engine.cpp
    investigation_view.cpp
    line_count_filter.cpp
    log_level_filter.cpp
)

target_link_libraries(scope_investigation
    PUBLIC
        scope_analysis
        scope_foundation
        scope_runtime
)

target_include_directories(scope_investigation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
