/**
 * @file report_output_test.cpp
 * @brief Unit tests for CLI report formatting.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "report_output.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::cli::OutputFormat;
using scope::cli::formatAnalysisOutput;
using scope::foundation::Path;

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
    const std::string output = formatAnalysisOutput(createSampleModel(), OutputFormat::Text);

    EXPECT_NE(std::string::npos, output.find("========== LOGSCOPE REPORT =========="));
    EXPECT_NE(std::string::npos, output.find("Total log lines : 3"));
    EXPECT_NE(std::string::npos, output.find("Error lines     : 1"));
}

TEST(ReportOutputTest, FormatsJsonReport)
{
    const std::string output = formatAnalysisOutput(createSampleModel(), OutputFormat::Json);

    EXPECT_NE(std::string::npos, output.find("\"source\": \"sample.log\""));
    EXPECT_NE(std::string::npos, output.find("\"totalLines\": 3"));
    EXPECT_NE(std::string::npos, output.find("\"errorLines\": 1"));
    EXPECT_NE(std::string::npos, output.find("\"warningLines\": 1"));
    EXPECT_NE(std::string::npos, output.find("\"infoLines\": 1"));
}
