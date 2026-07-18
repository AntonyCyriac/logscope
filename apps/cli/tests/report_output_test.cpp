/**
 * @file report_output_test.cpp
 * @brief Unit tests for CLI report formatting.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "report_output.hpp"

using scope::analysis::AnalysisModel;
using scope::cli::OutputFormat;
using scope::cli::formatAnalysisOutput;
using scope::foundation::Path;

TEST(ReportOutputTest, FormatsTextReport)
{
    const AnalysisModel model(Path("sample.log"), 8U);

    const std::string output = formatAnalysisOutput(model, OutputFormat::Text);

    EXPECT_NE(std::string::npos, output.find("========== LOGSCOPE REPORT =========="));
    EXPECT_NE(std::string::npos, output.find("Total log lines : 8"));
}

TEST(ReportOutputTest, FormatsJsonReport)
{
    const AnalysisModel model(Path("sample.log"), 8U);

    const std::string output = formatAnalysisOutput(model, OutputFormat::Json);

    EXPECT_NE(std::string::npos, output.find("\"source\": \"sample.log\""));
    EXPECT_NE(std::string::npos, output.find("\"totalLines\": 8"));
}
