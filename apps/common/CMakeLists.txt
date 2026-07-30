# ==========================================================
# LogScope Application Layer (shared CLI + desktop)
# ==========================================================

add_library(scope_application
    application_service.cpp
)

target_link_libraries(scope_application
    PUBLIC
        scope_ai
        scope_analysis
        scope_analytics
        scope_configuration
        scope_extension
        scope_foundation
        scope_investigation
        scope_plugin
        scope_query
        scope_reporting
        scope_runtime
        scope_search
        scope_source
        scope_storage
        scope_workspace
)

target_include_directories(scope_application
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
