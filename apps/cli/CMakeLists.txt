# ==========================================================
# LogScope CLI Application
# ==========================================================

add_executable(logscope
    LogAnalyzer.cpp
    analyze_command.cpp
    cli_application.cpp
    cli_config.cpp
    cli_parser.cpp
    config_validate_command.cpp
    extensions_command.cpp
    main.cpp
    output_format.cpp
    report_config.cpp
    report_output.cpp
)

target_link_libraries(logscope
    PRIVATE
        scope_analysis
        scope_configuration
        scope_extension
        scope_foundation
        scope_investigation
        scope_reporting
        scope_runtime
        scope_source
)

target_include_directories(logscope
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
