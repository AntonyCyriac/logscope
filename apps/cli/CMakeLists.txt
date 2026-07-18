# ==========================================================
# LogScope CLI Application
# ==========================================================

add_executable(logscope
    LogAnalyzer.cpp
    main.cpp
)

target_link_libraries(logscope
    PRIVATE
        scope_configuration
        scope_foundation
        scope_runtime
)

target_include_directories(logscope
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)
