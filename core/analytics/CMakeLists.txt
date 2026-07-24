# ==========================================================
# LogScope
# Core Analytics Engine
# ==========================================================

add_library(scope_analytics
    analytics_engine.cpp
    analytics_result.cpp
    correlation_analyzer.cpp
    correlation_result.cpp
    error_cluster.cpp
    error_clusterer.cpp
    frequency_analyzer.cpp
    frequency_result.cpp
    message_signature.cpp
    timeline_analyzer.cpp
    timeline_result.cpp
    trend_analyzer.cpp
    trend_result.cpp
)

target_link_libraries(scope_analytics
    PUBLIC
        scope_analysis
        scope_foundation
        scope_runtime
)

target_include_directories(scope_analytics
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
