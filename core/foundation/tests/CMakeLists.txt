add_executable(scope_foundation_status_test
    status_test.cpp
)

target_link_libraries(scope_foundation_status_test
    PRIVATE
        scope_foundation
)

add_test(
    NAME Foundation.Status.Basic
    COMMAND scope_foundation_status_test
)
