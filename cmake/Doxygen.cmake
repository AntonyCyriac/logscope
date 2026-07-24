# ==========================================================
# LogScope
# Doxygen API documentation target
# ==========================================================

find_package(Doxygen)

if(NOT DOXYGEN_FOUND)
    message(STATUS "Doxygen not found; 'docs' target unavailable (install Doxygen or set LOGSCOPE_DOCS=OFF)")
    return()
endif()

set(DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs/api")
set(DOXYGEN_MAINPAGE "${CMAKE_SOURCE_DIR}/docs/api/mainpage.md")

file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" DOXYGEN_SOURCE_DIR)
string(REPLACE "\\" "/" DOXYGEN_SOURCE_DIR "${DOXYGEN_SOURCE_DIR}")

configure_file(
    "${CMAKE_SOURCE_DIR}/docs/api/Doxyfile.in"
    "${CMAKE_BINARY_DIR}/Doxyfile"
    @ONLY
)

add_custom_target(docs
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DOXYGEN_OUTPUT_DIR}"
    COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/Doxyfile"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating LogScope API documentation with Doxygen"
    VERBATIM
)

message(STATUS "Doxygen 'docs' target enabled (output: ${DOXYGEN_OUTPUT_DIR}/html)")
