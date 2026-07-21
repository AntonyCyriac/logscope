# ==========================================================
# LogScope
# Core Search Engine
# ==========================================================

add_library(scope_search
    search_config.cpp
    search_engine.cpp
    search_history.cpp
    search_query.cpp
    search_query_parser.cpp
    text_matcher.cpp
)

target_link_libraries(scope_search
    PUBLIC
        scope_analysis
        scope_foundation
        scope_runtime
)

target_include_directories(scope_search
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
