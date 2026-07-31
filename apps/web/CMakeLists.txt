# ==========================================================
# LogScope Web Platform (M15.1 / C12)
# ==========================================================

include(${CMAKE_SOURCE_DIR}/cmake/LogScopeHttplib.cmake)

set(LOGSCOPE_WEB_CLI_SOURCES
    ../cli/investigation_output.cpp
    ../cli/report_output.cpp
    ../cli/report_config.cpp
    ../cli/output_format.cpp
    ../cli/cli_analysis_config.cpp
    ../cli/analytics_output.cpp
)

add_executable(logscope-web
    main.cpp
    web_config.cpp
    session_store.cpp
    json_parse.cpp
    rest_json.cpp
    web_request_parsers.cpp
    web_server.cpp
    middleware/api_key.cpp
    ${LOGSCOPE_WEB_CLI_SOURCES}
)

target_compile_definitions(logscope-web
    PRIVATE
        LOGSCOPE_VERSION="${PROJECT_VERSION}"
        "LOGSCOPE_WEB_UI_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/ui/dist\""
)

target_link_libraries(logscope-web
    PRIVATE
        scope_application
        logscope_httplib
)

target_include_directories(logscope-web
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../cli
)

if(WIN32)
    target_link_libraries(logscope-web PRIVATE ws2_32 crypt32)
endif()

install(TARGETS logscope-web
    RUNTIME DESTINATION bin
)

add_subdirectory(tests)
