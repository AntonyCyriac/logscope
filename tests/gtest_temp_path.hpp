/**
 * @file gtest_temp_path.hpp
 * @brief Unique per-test paths for parallel ctest runs.
 */

#pragma once

#include <string>

#include <gtest/gtest.h>

namespace logscope::gtest
{

inline std::string currentTestName()
{
    const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();

    return info != nullptr ? std::string(info->name()) : "unknown_test";
}

inline std::string uniqueTestPath(const std::string& suffix)
{
    return "logscope_test_" + currentTestName() + suffix;
}

} // namespace logscope::gtest
