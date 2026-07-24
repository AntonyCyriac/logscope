/**
 * @file report_output_test.cpp
 * @brief Unit tests for CLI report formatting.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "report_output.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::cli::generateAnalysisReport;
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
    options.sections = ReportSections::parse("summary,levels").value();

    const auto report = generateAnalysisReport(createSampleModel(), options);

    EXPECT_NE(std::string::npos, report.text().find("========== LOGSCOPE REPORT =========="));
    EXPECT_NE(std::string::npos, report.text().find("Total log lines : 3"));
    EXPECT_NE(std::string::npos, report.text().find("Error lines     : 1"));
}

TEST(ReportOutputTest, FormatsJsonReport)
{
    ReportOptions options;
    options.format = ReportFormat::Json;
    options.sections = ReportSections::parse("summary,levels").value();

    const auto report = generateAnalysisReport(createSampleModel(), options);

    EXPECT_NE(std::string::npos, report.text().find("\"summary\""));
    EXPECT_NE(std::string::npos, report.text().find("\"totalLines\": 3"));
    EXPECT_NE(std::string::npos, report.text().find("\"errorLines\": 1"));
}

TEST(ReportOutputTest, FormatsHtmlReport)
{
    ReportOptions options;
    options.format = ReportFormat::Html;
    options.sections = ReportSections::parse("executive,summary").value();

    const auto report = generateAnalysisReport(createSampleModel(), options);

    EXPECT_NE(std::string::npos, report.text().find("<!DOCTYPE html>"));
    EXPECT_NE(std::string::npos, report.text().find("Executive Summary"));
    EXPECT_NE(std::string::npos, report.text().find("<table>"));
}

TEST(ReportOutputTest, FormatsSummaryOnlySections)
{
    ReportOptions options;
    options.sections = ReportSections::parse("summary").value();

    const auto report = generateAnalysisReport(createSampleModel(), options);

    EXPECT_NE(std::string::npos, report.text().find("Total log lines : 3"));
    EXPECT_EQ(std::string::npos, report.text().find("--- Level Breakdown ---"));
}
