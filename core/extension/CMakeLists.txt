# ==========================================================
# LogScope
# Core Extension Manager
# ==========================================================

add_library(scope_extension
    builtin_extensions.cpp
    extension_manager.cpp
)

target_link_libraries(scope_extension
    PUBLIC
        scope_foundation
        scope_runtime
)

target_include_directories(scope_extension
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
