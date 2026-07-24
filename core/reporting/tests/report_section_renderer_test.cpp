/**
 * @file report_section_renderer_test.cpp
 * @brief Unit tests for ReportSectionRegistry.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "report_options.hpp"
#include "report_section_renderer.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::foundation::Path;
using scope::reporting::ReportSection;
using scope::reporting::ReportSectionRegistry;
using scope::reporting::ReportSections;

namespace
{

AnalysisModel createSampleModel()
{
    LogLevelCounts levelCounts;
    levelCounts.recordInfo();
    levelCounts.recordWarn();
    levelCounts.recordError();

    return AnalysisModel(Path("sample.log"), 3U, levelCounts);
}

} // namespace

TEST(ReportSectionRendererTest, RegistryDispatchesBuiltInSections)
{
    ReportSections sections;
    sections.enable(ReportSection::ExecutiveSummary);
    sections.enable(ReportSection::Summary);
    sections.enable(ReportSection::ErrorSummary);
    sections.enable(ReportSection::Charts);

    scope::reporting::ReportOptions options;
    options.sections = sections;

    const auto fragments = ReportSectionRegistry::instance().renderFragments(createSampleModel(), options);

    ASSERT_EQ(4U, fragments.size());
    EXPECT_FALSE(fragments[0U].textBody.empty());
    EXPECT_NE(std::string::npos, fragments[0U].textBody.find("Executive Summary"));
    EXPECT_NE(std::string::npos, fragments[2U].textBody.find("Error Summary"));
    EXPECT_NE(std::string::npos, fragments[3U].textBody.find("Charts"));
}
