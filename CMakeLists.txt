cmake_minimum_required(VERSION 3.20)

project(LogScope
    VERSION 0.1.0
    DESCRIPTION "Generic, extensible and high-performance log analysis platform"
    LANGUAGES CXX
)

# ----------------------------------------------------------
# C++ Standard
# ----------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------------------------------------
# Dependencies
# ----------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# ----------------------------------------------------------
# Testing
# ----------------------------------------------------------

enable_testing()

# ----------------------------------------------------------
# Workspace
# ----------------------------------------------------------

add_subdirectory(core)
add_subdirectory(apps)
add_subdirectory(tests)

# ----------------------------------------------------------
# Formatting
# ----------------------------------------------------------

include(cmake/Format.cmake)
