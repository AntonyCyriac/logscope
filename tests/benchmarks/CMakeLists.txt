# ==========================================================
# LogScope
# Performance Benchmarks
# ==========================================================

add_executable(logscope_benchmarks
    analysis_benchmark.cpp
)

target_link_libraries(logscope_benchmarks
    PRIVATE
        benchmark::benchmark
        benchmark_main
        scope_analysis
        scope_source
        scope_foundation
)

target_include_directories(logscope_benchmarks
    PRIVATE
        ${CMAKE_SOURCE_DIR}/core/analysis
        ${CMAKE_SOURCE_DIR}/core/source
        ${CMAKE_SOURCE_DIR}/core/foundation
)
