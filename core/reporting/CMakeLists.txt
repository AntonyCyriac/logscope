# ==========================================================
# LogScope
# Core Reporting Engine
# ==========================================================

add_library(scope_reporting
    report.cpp
    report_generator.cpp
)

target_link_libraries(scope_reporting
    PUBLIC
        scope_analysis
        scope_foundation
        scope_runtime
)

target_include_directories(scope_reporting
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
