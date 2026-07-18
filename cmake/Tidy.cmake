# ==========================================================
# LogScope
# clang-tidy target
# ==========================================================

find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)

if(CLANG_TIDY_EXECUTABLE)
    file(GLOB_RECURSE TIDY_SOURCE_FILES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/core/*"
        "${CMAKE_SOURCE_DIR}/apps/*"
    )

    list(FILTER TIDY_SOURCE_FILES EXCLUDE REGEX "/tests/")

    add_custom_target(tidy
        COMMAND ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR} ${TIDY_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on core and apps sources"
    )
else()
    message(STATUS "clang-tidy not found; 'tidy' target unavailable")
endif()
