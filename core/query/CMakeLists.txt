# ==========================================================
# LogScope
# Core Query Language
# ==========================================================

add_library(scope_query
    query_config.cpp
    query_evaluator.cpp
    query_node.cpp
    query_parser.cpp
    query_value.cpp
)

target_link_libraries(scope_query
    PUBLIC
        scope_analysis
        scope_foundation
        scope_runtime
)

target_include_directories(scope_query
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
