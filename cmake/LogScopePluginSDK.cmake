# ==========================================================
# LogScope Plugin SDK helpers (M12)
# ==========================================================

function(logscope_set_flat_output_directory target output_dir)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${output_dir}"
        LIBRARY_OUTPUT_DIRECTORY "${output_dir}"
        ARCHIVE_OUTPUT_DIRECTORY "${output_dir}"
    )

    if(CMAKE_CONFIGURATION_TYPES)
        foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
            string(TOUPPER "${config}" config_upper)
            set_target_properties(${target} PROPERTIES
                "RUNTIME_OUTPUT_DIRECTORY_${config_upper}" "${output_dir}"
                "LIBRARY_OUTPUT_DIRECTORY_${config_upper}" "${output_dir}"
                "ARCHIVE_OUTPUT_DIRECTORY_${config_upper}" "${output_dir}"
            )
        endforeach()
    endif()
endfunction()

function(logscope_add_plugin target)
    cmake_parse_arguments(ARG "" "" "SOURCES;LINK_LIBS" ${ARGN})

    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "logscope_add_plugin(${target}) requires SOURCES")
    endif()

    if(NOT ARG_LINK_LIBS)
        set(ARG_LINK_LIBS "")
    endif()

    add_library(${target} SHARED ${ARG_SOURCES})

    set_target_properties(${target} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

    target_compile_definitions(${target} PRIVATE LOGSCOPE_PLUGIN_EXPORT)

    target_link_libraries(${target} PRIVATE logscope_plugin_sdk)

    if(ARG_LINK_LIBS)
        target_link_libraries(${target} PRIVATE ${ARG_LINK_LIBS})
    endif()

    logscope_set_flat_output_directory(${target} "${CMAKE_CURRENT_BINARY_DIR}")
endfunction()
