# ==========================================================
# LogScope Applications
# ==========================================================

add_subdirectory(common)
add_subdirectory(cli)

if(LOGSCOPE_DESKTOP)
    add_subdirectory(desktop)
endif()
