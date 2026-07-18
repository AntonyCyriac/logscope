/**
 * @file diagnostics_test.cpp
 * @brief Unit tests for Diagnostics.
 */

#include <gtest/gtest.h>

#include "runtime.hpp"

using scope::runtime::Diagnostics;
using scope::runtime::LogLevel;

TEST(DiagnosticsTest, Instance)
{
    Diagnostics& first = Diagnostics::instance();
    Diagnostics& second = Diagnostics::instance();

    EXPECT_EQ(&first, &second);
}

TEST(DiagnosticsTest, LogDoesNotThrow)
{
    EXPECT_NO_THROW(Diagnostics::instance().log(LogLevel::Info, "test message"));
}
