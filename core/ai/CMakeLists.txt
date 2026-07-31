# ==========================================================
# LogScope
# Core AI Assistant
# ==========================================================

include(${CMAKE_SOURCE_DIR}/cmake/LogScopeHttplib.cmake)

add_library(scope_ai
    ai_analytics_context_builder.cpp
    ai_anomaly_hint_formatter.cpp
    ai_config.cpp
    ai_context_builder.cpp
    ai_investigation_assistant.cpp
    ai_json_util.cpp
    ai_provider.cpp
    ai_summary_formatter.cpp
    http_ai_client.cpp
    http_ai_provider.cpp
    nl_query_translator.cpp
    noop_ai_provider.cpp
)

target_link_libraries(scope_ai
    PUBLIC
        scope_analytics
        scope_foundation
        scope_investigation
        scope_query
        scope_runtime
    PRIVATE
        logscope_httplib
)

if(WIN32)
    target_link_libraries(scope_ai PRIVATE ws2_32 crypt32)
endif()

target_include_directories(scope_ai
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
