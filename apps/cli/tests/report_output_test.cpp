/**
 * @file report_output_test.cpp
 * @brief Unit tests for CLI report formatting.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "report_output.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::cli::formatAnalysisOutput;
using scope::foundation::Path;
using scope::reporting::ReportFormat;
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

} // namespace

TEST(ReportOutputTest, FormatsTextReport)
{
    ReportOptions options;

    const std::string output = formatAnalysisOutput(createSampleModel(), options);

    EXPECT_NE(std::string::npos, output.find("========== LOGSCOPE REPORT =========="));
    EXPECT_NE(std::string::npos, output.find("Total log lines : 3"));
    EXPECT_NE(std::string::npos, output.find("Error lines     : 1"));
}

TEST(ReportOutputTest, FormatsJsonReport)
{
    ReportOptions options;
    options.format = ReportFormat::Json;

    const std::string output = formatAnalysisOutput(createSampleModel(), options);

    EXPECT_NE(std::string::npos, output.find("\"summary\""));
    EXPECT_NE(std::string::npos, output.find("\"totalLines\": 3"));
    EXPECT_NE(std::string::npos, output.find("\"errorLines\": 1"));
    EXPECT_NE(std::string::npos, output.find("\"warningLines\": 1"));
    EXPECT_NE(std::string::npos, output.find("\"infoLines\": 1"));
}

TEST(ReportOutputTest, FormatsSummaryOnlySections)
{
    ReportOptions options;
    options.sections = ReportSections::parse("summary").value();

    const std::string output = formatAnalysisOutput(createSampleModel(), options);

    EXPECT_NE(std::string::npos, output.find("Total log lines : 3"));
    EXPECT_EQ(std::string::npos, output.find("--- Level Breakdown ---"));
}
