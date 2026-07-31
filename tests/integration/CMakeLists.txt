add_executable(logscope_integration_tests
    pipeline_integration_test.cpp
)

target_link_libraries(logscope_integration_tests
    PRIVATE
        logscope_gtest_support
        scope_workspace
        scope_configuration
        scope_extension
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(logscope_integration_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

if(LOGSCOPE_WEB)
    add_subdirectory(web)
endif()
