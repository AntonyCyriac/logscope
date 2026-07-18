add_executable(logscope_e2e_tests
    cli_e2e_test.cpp
)

add_dependencies(logscope_e2e_tests logscope)

target_compile_definitions(logscope_e2e_tests
    PRIVATE
        "LOGSCOPE_EXECUTABLE=R\"($<TARGET_FILE:logscope>)\""
        "LOGSCOPE_SOURCE_DIR=R\"(${CMAKE_SOURCE_DIR})\""
)

target_link_libraries(logscope_e2e_tests
    PRIVATE
        GTest::gtest_main
)

target_include_directories(logscope_e2e_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

include(GoogleTest)

gtest_discover_tests(logscope_e2e_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
