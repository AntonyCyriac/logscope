# ==========================================================
# LogScope GTest support (parallel-safe temp paths)
# ==========================================================

add_library(logscope_gtest_support INTERFACE)
target_include_directories(logscope_gtest_support INTERFACE ${CMAKE_SOURCE_DIR}/tests)
target_link_libraries(logscope_gtest_support INTERFACE GTest::gtest)
