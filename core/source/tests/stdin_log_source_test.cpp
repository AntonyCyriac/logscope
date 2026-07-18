/**
 * @file stdin_log_source_test.cpp
 * @brief Unit tests for StdinLogSource.
 */

#include <gtest/gtest.h>

#include "stdin_log_source.hpp"

using scope::foundation::Path;
using scope::source::StdinLogSource;

TEST(StdinLogSourceTest, OpenProvidesStdinPath)
{
    const auto result = StdinLogSource::open();

    ASSERT_TRUE(result.hasValue());
    EXPECT_EQ("-", (*result)->path().string());
}
