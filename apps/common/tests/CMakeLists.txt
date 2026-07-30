# ==========================================================
# Application layer tests
# ==========================================================

add_executable(scope_application_tests
    application_service_test.cpp
)

target_link_libraries(scope_application_tests
    PRIVATE
        scope_application
        logscope_gtest_support
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(scope_application_tests
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    PROPERTIES LABELS "scope_application_tests"
)
