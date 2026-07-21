# ==========================================================
# LogScope CLI Application
# ==========================================================

add_executable(logscope
    log_analyzer.cpp
    analyze_command.cpp
    investigate_command.cpp
    investigation_output.cpp
    cli_application.cpp
    cli_config.cpp
    cli_parser.cpp
    config_validate_command.cpp
    extensions_command.cpp
    main.cpp
    session_command.cpp
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
        scope_workspace
)

target_include_directories(logscope
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS logscope
    RUNTIME DESTINATION bin
)

add_subdirectory(tests)
