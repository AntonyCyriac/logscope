include(${CMAKE_SOURCE_DIR}/cmake/LogScopePluginSDK.cmake)

logscope_add_plugin(sample_storage_plugin SOURCES plugin.cpp LINK_LIBS scope_foundation)
