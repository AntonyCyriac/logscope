# ==========================================================
# LogScope
# clang-format target
# ==========================================================

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)

if(CLANG_FORMAT_EXECUTABLE)
    file(GLOB_RECURSE FORMAT_SOURCE_FILES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/core/*"
        "${CMAKE_SOURCE_DIR}/apps/*"
    )

    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${FORMAT_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting source files with clang-format"
    )
else()
    message(STATUS "clang-format not found; 'format' target unavailable")
endif()
