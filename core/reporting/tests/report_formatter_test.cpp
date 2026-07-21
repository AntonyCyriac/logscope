/**
 * @file report_formatter_test.cpp
 * @brief Unit tests for ReportFormatter.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "report_formatter.hpp"
#include "report_options.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::foundation::Path;
using scope::reporting::ReportFormat;
using scope::reporting::ReportFormatter;
using scope::reporting::ReportOptions;
using scope::reporting::ReportSection;
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

ReportOptions optionsWithSections(const ReportSections& sections, const ReportFormat format)
{
    ReportOptions options;
    options.sections = sections;
    options.format = format;

    return options;
}

} // namespace

TEST(ReportFormatterTest, FormatsCsvWithSelectedSections)
{
    ReportSections sections;
    sections.enable(ReportSection::Summary);

    const auto report = ReportFormatter::format(createSampleModel(),
                                                optionsWithSections(sections, ReportFormat::Csv));

    EXPECT_NE(std::string::npos, report.text().find("section,key,value"));
    EXPECT_NE(std::string::npos, report.text().find("summary,totalLines,3"));
    EXPECT_EQ(std::string::npos, report.text().find("levelBreakdown"));
}

TEST(ReportFormatterTest, FormatsMarkdownWithAllSections)
{
    const auto report =
        ReportFormatter::format(createSampleModel(), optionsWithSections(ReportSections::all(), ReportFormat::Markdown));

    EXPECT_NE(std::string::npos, report.text().find("# LogScope Report"));
    EXPECT_NE(std::string::npos, report.text().find("## Summary"));
    EXPECT_NE(std::string::npos, report.text().find("## Level Breakdown"));
    EXPECT_NE(std::string::npos, report.text().find("## Source Metadata"));
}

TEST(ReportFormatterTest, FormatsJsonWithSectionStructure)
{
    const auto report =
        ReportFormatter::format(createSampleModel(), optionsWithSections(ReportSections::all(), ReportFormat::Json));

    EXPECT_NE(std::string::npos, report.text().find("\"summary\""));
    EXPECT_NE(std::string::npos, report.text().find("\"totalLines\": 3"));
    EXPECT_NE(std::string::npos, report.text().find("\"levelBreakdown\""));
    EXPECT_NE(std::string::npos, report.text().find("\"sourceMetadata\""));
    EXPECT_NE(std::string::npos, report.text().find("\"format\""));
}

TEST(ReportFormatterTest, IncludesJsonLinesSummaryInSourceMetadata)
{
    scope::analysis::JsonLinesSummary summary;
    summary.recordValidLine({"level", "message"});

    const AnalysisModel model(Path("sample.jsonl"),
                              2U,
                              {},
                              scope::analysis::LogFormat::JsonLines,
                              summary);

    const auto report =
        ReportFormatter::format(model, optionsWithSections(ReportSections::all(), ReportFormat::Json));

    EXPECT_NE(std::string::npos, report.text().find("\"jsonValidLines\": 1"));
    EXPECT_NE(std::string::npos, report.text().find("\"topLevelKeys\""));
}
