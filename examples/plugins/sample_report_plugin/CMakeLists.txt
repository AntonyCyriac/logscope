include(${CMAKE_SOURCE_DIR}/cmake/LogScopePluginSDK.cmake)

logscope_add_plugin(sample_report_plugin SOURCES plugin.cpp LINK_LIBS)
