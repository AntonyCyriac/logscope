# ==========================================================
# REST vs CLI parity tests (M15.1)
# ==========================================================

include(${CMAKE_SOURCE_DIR}/cmake/LogScopeHttplib.cmake)
include(${CMAKE_SOURCE_DIR}/apps/web/cmake/LogScopeWebOpenSSL.cmake)

add_executable(logscope_web_parity_tests
    cli_rest_parity_test.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/web_server.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/web_config.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/session_store.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/session_resource_cleanup.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/workspace_store.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/investigation_store.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/analyze_job_queue.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/json_parse.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/rest_json.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/web_request_parsers.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/middleware/api_key.cpp
    ${CMAKE_SOURCE_DIR}/apps/web/middleware/api_key_credential.cpp
    ${CMAKE_SOURCE_DIR}/apps/cli/investigation_output.cpp
    ${CMAKE_SOURCE_DIR}/apps/cli/report_output.cpp
    ${CMAKE_SOURCE_DIR}/apps/cli/report_config.cpp
    ${CMAKE_SOURCE_DIR}/apps/cli/output_format.cpp
    ${CMAKE_SOURCE_DIR}/apps/cli/cli_analysis_config.cpp
    ${CMAKE_SOURCE_DIR}/apps/cli/analytics_output.cpp
)

add_dependencies(logscope_web_parity_tests logscope logscope-web)

target_compile_definitions(logscope_web_parity_tests
    PRIVATE
        "LOGSCOPE_EXECUTABLE=R\"($<TARGET_FILE:logscope>)\""
        "LOGSCOPE_WEB_EXECUTABLE=R\"($<TARGET_FILE:logscope-web>)\""
        "LOGSCOPE_SOURCE_DIR=R\"(${CMAKE_SOURCE_DIR})\""
        LOGSCOPE_VERSION="${PROJECT_VERSION}"
        "LOGSCOPE_WEB_UI_DIR=\"${CMAKE_SOURCE_DIR}/apps/web/ui/dist\""
)

target_link_libraries(logscope_web_parity_tests
    PRIVATE
        scope_application
        scope_configuration
        scope_workspace
        logscope_httplib
        logscope_gtest_support
        GTest::gtest_main
)

target_include_directories(logscope_web_parity_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/tests/end_to_end
        ${CMAKE_SOURCE_DIR}/apps/web
        ${CMAKE_SOURCE_DIR}/apps/cli
)

if(WIN32)
    target_link_libraries(logscope_web_parity_tests PRIVATE ws2_32 crypt32)
endif()

logscope_web_link_openssl(logscope_web_parity_tests)

include(GoogleTest)
gtest_discover_tests(logscope_web_parity_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES LABELS "logscope_web_integration_tests" TIMEOUT 120
)
