# ==========================================================
# LogScope
# Core Configuration Manager
# ==========================================================

add_library(scope_configuration
    configuration_manager.cpp
)

target_link_libraries(scope_configuration
    PUBLIC
        scope_foundation
        scope_runtime
)

target_include_directories(scope_configuration
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
