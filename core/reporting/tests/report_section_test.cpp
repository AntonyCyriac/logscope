/**
 * @file report_section_test.cpp
 * @brief Unit tests for ReportSections.
 */

#include <gtest/gtest.h>

#include "report_section.hpp"

using scope::reporting::ReportSection;
using scope::reporting::ReportSections;

TEST(ReportSectionTest, AllIncludesEverySection)
{
    const ReportSections sections = ReportSections::all();

    EXPECT_TRUE(sections.includes(ReportSection::Summary));
    EXPECT_TRUE(sections.includes(ReportSection::LevelBreakdown));
    EXPECT_TRUE(sections.includes(ReportSection::SourceMetadata));
}

TEST(ReportSectionTest, ParsesCommaSeparatedNames)
{
    const auto sections = ReportSections::parse("summary,levels,metadata");

    ASSERT_TRUE(sections);
    EXPECT_TRUE(sections->includes(ReportSection::Summary));
    EXPECT_TRUE(sections->includes(ReportSection::LevelBreakdown));
    EXPECT_TRUE(sections->includes(ReportSection::SourceMetadata));
}

TEST(ReportSectionTest, ParsesAllKeyword)
{
    const auto sections = ReportSections::parse("all");

    ASSERT_TRUE(sections);
    EXPECT_TRUE(sections->includes(ReportSection::Summary));
    EXPECT_TRUE(sections->includes(ReportSection::LevelBreakdown));
    EXPECT_TRUE(sections->includes(ReportSection::SourceMetadata));
}

TEST(ReportSectionTest, RejectsUnknownSection)
{
    EXPECT_FALSE(ReportSections::parse("summary,unknown"));
}
