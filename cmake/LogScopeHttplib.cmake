# ==========================================================
# Shared cpp-httplib dependency (scope_ai + logscope-web)
# ==========================================================

include_guard(GLOBAL)

if(TARGET logscope_httplib)
    return()
endif()

include(FetchContent)

FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.14.3
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(httplib)

add_library(logscope_httplib INTERFACE)
target_link_libraries(logscope_httplib INTERFACE httplib::httplib)
