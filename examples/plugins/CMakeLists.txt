# ==========================================================
# LogScope Sample Plugins (M12)
# ==========================================================

option(LOGSCOPE_BUILD_SAMPLE_PLUGINS "Build example dynamic plugins" ON)

if(LOGSCOPE_BUILD_SAMPLE_PLUGINS)
    include(${CMAKE_SOURCE_DIR}/cmake/LogScopePluginSDK.cmake)

    add_subdirectory(sample_report_plugin)
    add_subdirectory(sample_parser_plugin)
    add_subdirectory(sample_search_plugin)
    add_subdirectory(sample_storage_plugin)
endif()
