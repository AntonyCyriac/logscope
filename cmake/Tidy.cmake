# ==========================================================
# LogScope
# clang-tidy target
# ==========================================================

find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy)

if(CLANG_TIDY_EXECUTABLE)
    file(GLOB_RECURSE TIDY_CPP_FILES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/core/*.cpp"
        "${CMAKE_SOURCE_DIR}/apps/*.cpp"
    )

    file(GLOB_RECURSE TIDY_HEADER_FILES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/core/*.hpp"
        "${CMAKE_SOURCE_DIR}/apps/*.hpp"
    )

    set(TIDY_SOURCE_FILES ${TIDY_CPP_FILES} ${TIDY_HEADER_FILES})
    list(FILTER TIDY_SOURCE_FILES EXCLUDE REGEX "/tests/")

    add_custom_target(tidy
        COMMAND ${CLANG_TIDY_EXECUTABLE} -p ${CMAKE_BINARY_DIR} ${TIDY_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy on core and apps C++ sources"
    )
else()
    message(STATUS "clang-tidy not found; 'tidy' target unavailable")
endif()
