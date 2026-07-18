# ==========================================================
# LogScope
# Core Runtime (Platform Services)
# ==========================================================

add_library(scope_runtime
    configuration.cpp
    diagnostics.cpp
    plugin_registry.cpp
    service_registry.cpp
)

target_link_libraries(scope_runtime
    PUBLIC
        scope_foundation
)

target_include_directories(scope_runtime
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
