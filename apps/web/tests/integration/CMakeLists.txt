# ==========================================================
# Web platform integration tests (M15.1)
# ==========================================================

add_executable(logscope_web_integration_tests
    web_api_integration_test.cpp
    ../../web_server.cpp
    ../../web_config.cpp
    ../../session_store.cpp
    ../../workspace_store.cpp
    ../../analyze_job_queue.cpp
    ../../json_parse.cpp
    ../../rest_json.cpp
    ../../web_request_parsers.cpp
    ../../middleware/api_key.cpp
    ../../../cli/investigation_output.cpp
    ../../../cli/report_output.cpp
    ../../../cli/report_config.cpp
    ../../../cli/output_format.cpp
    ../../../cli/cli_analysis_config.cpp
    ../../../cli/analytics_output.cpp
)

target_compile_definitions(logscope_web_integration_tests
    PRIVATE
        LOGSCOPE_VERSION="${PROJECT_VERSION}"
        "LOGSCOPE_WEB_UI_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../../ui/dist\""
        "LOGSCOPE_SOURCE_DIR=R\"(${CMAKE_SOURCE_DIR})\""
)

target_link_libraries(logscope_web_integration_tests
    PRIVATE
        scope_application
        scope_configuration
        logscope_httplib
        logscope_gtest_support
        GTest::gtest_main
)

target_include_directories(logscope_web_integration_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../..
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../cli
)

if(WIN32)
    target_link_libraries(logscope_web_integration_tests PRIVATE ws2_32 crypt32)
endif()

include(GoogleTest)
gtest_discover_tests(logscope_web_integration_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES LABELS "logscope_web_integration_tests" TIMEOUT 120
)
