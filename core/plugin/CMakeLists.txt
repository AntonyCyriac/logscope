# ==========================================================
# LogScope
# Core Plugin Loader (M12)
# ==========================================================

add_library(logscope_plugin_sdk INTERFACE)

target_include_directories(logscope_plugin_sdk INTERFACE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/core/foundation
    ${CMAKE_SOURCE_DIR}/core/analysis
    ${CMAKE_SOURCE_DIR}/core/search
    ${CMAKE_SOURCE_DIR}/core/storage
    ${CMAKE_SOURCE_DIR}/core/reporting
    ${CMAKE_SOURCE_DIR}/core/investigation
    ${CMAKE_SOURCE_DIR}/core/runtime
    ${CMAKE_SOURCE_DIR}/core/source
)

add_library(scope_plugin
    plugin_config.cpp
    plugin_host_api.cpp
    plugin_loader.cpp
    plugin_runtime.cpp
    shared_library.cpp
)

target_link_libraries(scope_plugin
    PUBLIC
        scope_foundation
        scope_extension
        logscope_plugin_sdk
    PRIVATE
        scope_analysis
        scope_search
        scope_storage
        scope_reporting
)

target_include_directories(scope_plugin
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_subdirectory(tests)

install(TARGETS logscope_plugin_sdk scope_plugin
    EXPORT LogScopeTargets
)

install(FILES
    ${CMAKE_SOURCE_DIR}/include/logscope/plugin/plugin.h
    ${CMAKE_SOURCE_DIR}/include/logscope/plugin/plugin.hpp
    DESTINATION include/logscope/plugin
)

install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/LogScopePluginSDK.cmake
    DESTINATION lib/cmake/LogScope
)
