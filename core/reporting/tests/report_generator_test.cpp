/**
 * @file report_generator_test.cpp
 * @brief Unit tests for ReportGenerator.
 */

#include <gtest/gtest.h>

#include "reporting.hpp"

using scope::analysis::AnalysisModel;
using scope::foundation::Path;
using scope::reporting::Report;
using scope::reporting::ReportGenerator;

TEST(ReportGeneratorTest, GeneratesTextReport)
{
    const AnalysisModel model(Path("sample.log"), 8U);

    ReportGenerator generator;

    const Report report = generator.generate(model);

    EXPECT_NE(std::string::npos, report.text().find("========== LOGSCOPE REPORT =========="));
    EXPECT_NE(std::string::npos, report.text().find("Source          : sample.log"));
    EXPECT_NE(std::string::npos, report.text().find("Total log lines : 8"));
    EXPECT_NE(std::string::npos, report.text().find("====================================="));
}

TEST(ReportGeneratorTest, GeneratesReportForEmptyAnalysis)
{
    const AnalysisModel model(Path("empty.log"), 0U);

    ReportGenerator generator;

    const Report report = generator.generate(model);

    EXPECT_NE(std::string::npos, report.text().find("Total log lines : 0"));
}
