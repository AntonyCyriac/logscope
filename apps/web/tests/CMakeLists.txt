# ==========================================================
# Web platform unit tests (M15.1)
# ==========================================================

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/LogScopeWebOpenSSL.cmake)

option(LOGSCOPE_WEB_UI_BUILD "Build web UI assets with Vite (optional; dist/ is vendored)" OFF)

add_executable(scope_web_tests
    health_handler_test.cpp
    session_store_test.cpp
    session_resource_cleanup_test.cpp
    workspace_store_test.cpp
    investigation_store_test.cpp
    analyze_job_queue_test.cpp
    tail_handler_test.cpp
    web_config_test.cpp
    web_request_parsers_test.cpp
    rest_json_test.cpp
    api_key_middleware_test.cpp
    api_key_credential_test.cpp
    web_security_test.cpp
    upload_size_limit_test.cpp
    ../web_server.cpp
    ../web_config.cpp
    ../web_startup.cpp
    ../session_store.cpp
    ../session_resource_cleanup.cpp
    ../workspace_store.cpp
    ../investigation_store.cpp
    ../analyze_job_queue.cpp
    ../json_parse.cpp
    ../rest_json.cpp
    ../web_request_parsers.cpp
    ../middleware/api_key.cpp
    ../middleware/api_key_credential.cpp
    ../../cli/investigation_output.cpp
    ../../cli/report_output.cpp
    ../../cli/report_config.cpp
    ../../cli/output_format.cpp
    ../../cli/cli_analysis_config.cpp
    ../../cli/analytics_output.cpp
)

target_compile_definitions(scope_web_tests
    PRIVATE
        LOGSCOPE_VERSION="${PROJECT_VERSION}"
        "LOGSCOPE_WEB_UI_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../ui/dist\""
        "LOGSCOPE_SOURCE_DIR=R\"(${CMAKE_SOURCE_DIR})\""
)

target_link_libraries(scope_web_tests
    PRIVATE
        scope_application
        scope_configuration
        scope_workspace
        logscope_httplib
        logscope_gtest_support
        GTest::gtest_main
)

target_include_directories(scope_web_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../../cli
        ${CMAKE_SOURCE_DIR}/tests/end_to_end
)

if(WIN32)
    target_link_libraries(scope_web_tests PRIVATE ws2_32 crypt32)
endif()

logscope_web_link_openssl(scope_web_tests)

include(GoogleTest)
gtest_discover_tests(scope_web_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES LABELS "scope_web_tests"
)

if(LOGSCOPE_WEB)
    add_subdirectory(integration)
endif()
