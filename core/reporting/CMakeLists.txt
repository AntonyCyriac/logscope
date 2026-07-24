# ==========================================================
# LogScope
# Core Reporting Engine
# ==========================================================

add_library(scope_reporting
    ascii_chart_renderer.cpp
    chart_model.cpp
    format_renderer.cpp
    minimal_pdf_writer.cpp
    report.cpp
    report_analytics_sections.cpp
    report_builtin_sections.cpp
    report_contributors.cpp
    report_format.cpp
    report_formatter.cpp
    report_fragment.cpp
    report_generator.cpp
    report_options.cpp
    report_section.cpp
    report_section_renderer.cpp
    report_string_utils.cpp
    svg_chart_renderer.cpp
)

target_link_libraries(scope_reporting
    PUBLIC
        scope_analysis
        scope_analytics
        scope_foundation
        scope_runtime
)

target_include_directories(scope_reporting
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
