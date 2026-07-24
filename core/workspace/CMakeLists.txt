# ==========================================================
# LogScope
# Core Workspace
# ==========================================================

add_library(scope_workspace
    investigation_session.cpp
    session_serializer.cpp
    session_store.cpp
)

target_link_libraries(scope_workspace
    PUBLIC
        scope_analysis
        scope_foundation
        scope_investigation
        scope_query
        scope_reporting
        scope_runtime
        scope_search
        scope_storage
)

target_include_directories(scope_workspace
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
