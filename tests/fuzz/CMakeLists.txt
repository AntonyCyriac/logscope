# ==========================================================
# LogScope
# libFuzzer Targets (Clang only)
# ==========================================================

if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "LOGSCOPE_FUZZING requires Clang; fuzz targets skipped")
    return()
endif()

function(add_logscope_fuzz_target target_name source_file)
    add_executable(${target_name} ${source_file})

    target_compile_options(${target_name} PRIVATE -fsanitize=fuzzer)
    target_link_options(${target_name} PRIVATE -fsanitize=fuzzer)

    target_include_directories(${target_name}
        PRIVATE
            ${CMAKE_SOURCE_DIR}/core/workspace
            ${CMAKE_SOURCE_DIR}/core/configuration
            ${CMAKE_SOURCE_DIR}/core/foundation
    )
endfunction()

add_logscope_fuzz_target(session_serializer_fuzz session_serializer_fuzz.cpp)
target_link_libraries(session_serializer_fuzz PRIVATE scope_workspace)

add_logscope_fuzz_target(configuration_fuzz configuration_fuzz.cpp)
target_link_libraries(configuration_fuzz PRIVATE scope_configuration scope_foundation)
