# ==========================================================
# LogScope
# Core Analysis Engine
# ==========================================================

add_library(scope_analysis
    analysis_engine.cpp
    analysis_model.cpp
    log_level_counts.cpp
    log_line_classifier.cpp
)

target_link_libraries(scope_analysis
    PUBLIC
        scope_foundation
        scope_source
        scope_runtime
)

target_include_directories(scope_analysis
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
