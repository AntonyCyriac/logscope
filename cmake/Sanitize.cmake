# ==========================================================
# LogScope
# Sanitizer build options
# ==========================================================

option(LOGSCOPE_SANITIZE "Enable AddressSanitizer, LeakSanitizer, and UndefinedBehaviorSanitizer (GCC/Clang)" OFF)

if(LOGSCOPE_SANITIZE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address,undefined)
        message(STATUS "Sanitizers enabled: address, leak (via ASan), undefined")
    else()
        message(WARNING "LOGSCOPE_SANITIZE is only supported with Clang or GCC")
    endif()
endif()
