/**
 * @file process_memory_test.cpp
 */

#include <gtest/gtest.h>

#include "process_memory.hpp"

TEST(ProcessMemoryTest, CurrentProcessMemoryUsageReturnsRss)
{
    const auto usage = scope::foundation::currentProcessMemoryUsage();

    EXPECT_TRUE(usage.has_value());
    EXPECT_GT(usage->residentBytes, 0U);
}
