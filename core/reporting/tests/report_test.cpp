/**
 * @file report_test.cpp
 * @brief Unit tests for Report.
 */

#include <gtest/gtest.h>

#include "reporting.hpp"

using scope::reporting::Report;

TEST(ReportTest, StoresText)
{
    const Report report("report body");

    EXPECT_EQ("report body", report.text());
}
