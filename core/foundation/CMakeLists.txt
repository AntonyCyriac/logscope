# ==========================================================
# LogScope
# Core Foundation
# ==========================================================

add_library(scope_foundation
    error.cpp
    version.cpp
)

target_include_directories(scope_foundation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)
