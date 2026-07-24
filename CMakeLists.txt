cmake_minimum_required(VERSION 3.20)

project(LogScope
    VERSION 1.4.0
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
# Options
# ----------------------------------------------------------

option(LOGSCOPE_BENCHMARKS "Build performance benchmarks" OFF)
option(LOGSCOPE_FUZZING "Build libFuzzer targets (requires Clang)" OFF)

# ----------------------------------------------------------
# Toolchain helpers
# ----------------------------------------------------------

include(cmake/Sanitize.cmake)

# ----------------------------------------------------------
# Dependencies
# ----------------------------------------------------------

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

if(LOGSCOPE_BENCHMARKS)
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.9.1.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )

    FetchContent_MakeAvailable(googlebenchmark)
endif()

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
# Formatting and static analysis
# ----------------------------------------------------------

include(cmake/Format.cmake)
include(cmake/Tidy.cmake)

# ----------------------------------------------------------
# Packaging
# ----------------------------------------------------------

set(CPACK_PACKAGE_NAME "logscope")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

include(CPack)
