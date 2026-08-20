# ==========================================================
# LogScope
# Core Run Comparison (H0 Wave 4)
# ==========================================================

add_library(scope_compare
    comparison_result.cpp
    run_aligner.cpp
    run_comparison.cpp
    run_differ.cpp
    run_snapshot.cpp
)

target_link_libraries(scope_compare
    PUBLIC
        scope_analysis
        scope_analytics
        scope_foundation
        scope_source
)

target_include_directories(scope_compare
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
