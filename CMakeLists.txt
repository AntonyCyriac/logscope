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

enable_testing()

# ----------------------------------------------------------
# Workspace
# ----------------------------------------------------------

add_subdirectory(core)

# ----------------------------------------------------------
# Current executable (temporary)
# ----------------------------------------------------------

add_executable(logscope
    src/main.cpp
    src/LogAnalyzer.cpp
)

target_include_directories(logscope
    PRIVATE
        include
)
