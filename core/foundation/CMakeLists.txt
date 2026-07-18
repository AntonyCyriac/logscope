# ==========================================================
# LogScope
# Core Foundation
# ==========================================================

add_library(scope_foundation
    date.cpp
    datetime.cpp
    clock.cpp
    duration.cpp
    error.cpp
    filesystem.cpp
    hash.cpp
    path.cpp
    random.cpp
    stopwatch.cpp
    string.cpp
    time.cpp
    timestamp.cpp
    uuid.cpp
    version.cpp
)

target_include_directories(scope_foundation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

add_subdirectory(tests)
