# Links OpenSSL Crypto (API key hashing) and optional TLS for logscope-web targets.

function(logscope_web_link_openssl target)
    if(NOT OpenSSL_FOUND)
        find_package(OpenSSL)
    endif()

    if(OpenSSL_FOUND)
        target_compile_definitions(${target} PRIVATE LOGSCOPE_WEB_API_KEY_HASHING)
        target_link_libraries(${target} PRIVATE OpenSSL::Crypto)

        if(LOGSCOPE_WEB_TLS)
            target_compile_definitions(${target} PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT)
            target_link_libraries(${target} PRIVATE OpenSSL::SSL)
        endif()
    endif()
endfunction()
