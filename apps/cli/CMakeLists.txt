# ==========================================================
# LogScope CLI Application
# ==========================================================

add_executable(logscope
    log_analyzer.cpp
    agent_command.cpp
    analyze_command.cpp
    compare_command.cpp
    compare_output.cpp
    analytics_command.cpp
    analytics_output.cpp
    cli_analysis_config.cpp
    investigate_command.cpp
    query_command.cpp
    search_command.cpp
    investigation_output.cpp
    cli_application.cpp
    cli_config.cpp
    cli_parser.cpp
    config_validate_command.cpp
    extensions_command.cpp
    main.cpp
    session_command.cpp
    investigation_command.cpp
    stats_output.cpp
    output_format.cpp
    report_config.cpp
    report_output.cpp
    report_writer.cpp
)

target_link_libraries(logscope
    PRIVATE
        scope_application
        scope_analysis
        scope_analytics
        scope_compare
        scope_configuration
        scope_extension
        scope_plugin
        scope_ai
        scope_foundation
        scope_investigation
        scope_query
        scope_reporting
        scope_runtime
        scope_search
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

if(WIN32)
    target_link_libraries(logscope PRIVATE psapi)
endif()

add_subdirectory(tests)
