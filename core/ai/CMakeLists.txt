# ==========================================================
# LogScope
# Core AI Assistant
# ==========================================================

add_library(scope_ai
    ai_config.cpp
    ai_context_builder.cpp
    ai_investigation_assistant.cpp
    ai_provider.cpp
    ai_summary_formatter.cpp
    http_ai_provider.cpp
    nl_query_translator.cpp
    noop_ai_provider.cpp
)

target_link_libraries(scope_ai
    PUBLIC
        scope_foundation
        scope_investigation
        scope_query
        scope_runtime
)

target_include_directories(scope_ai
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
