# ==========================================================
# LogScope
# Repository Tests
# ==========================================================

add_subdirectory(integration)
add_subdirectory(end_to_end)
add_subdirectory(regression)

if(LOGSCOPE_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(LOGSCOPE_FUZZING)
    add_subdirectory(fuzz)
endif()
