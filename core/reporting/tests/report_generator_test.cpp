/**
 * @file report_generator_test.cpp
 * @brief Unit tests for ReportGenerator.
 */

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "reporting.hpp"

using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::foundation::Path;
using scope::reporting::Report;
using scope::reporting::ReportGenerator;

namespace
{

AnalysisModel createSampleModel()
{
    LogLevelCounts levelCounts;
    levelCounts.recordInfo();
    levelCounts.recordInfo();
    levelCounts.recordInfo();
    levelCounts.recordWarn();
    levelCounts.recordError();
    levelCounts.recordError();
    levelCounts.recordError();
    levelCounts.recordError();

    return AnalysisModel(Path("sample.log"), 8U, levelCounts);
}

} // namespace

TEST(ReportGeneratorTest, GeneratesTextReport)
{
    ReportGenerator generator;

    const Report report = generator.generate(createSampleModel());

    EXPECT_NE(std::string::npos, report.text().find("========== LOGSCOPE REPORT =========="));
    EXPECT_NE(std::string::npos, report.text().find("Source          : sample.log"));
    EXPECT_NE(std::string::npos, report.text().find("Total log lines : 8"));
    EXPECT_NE(std::string::npos, report.text().find("Error lines     : 4"));
    EXPECT_NE(std::string::npos, report.text().find("Warning lines   : 1"));
    EXPECT_NE(std::string::npos, report.text().find("Info lines      : 3"));
    EXPECT_NE(std::string::npos, report.text().find("====================================="));
}

TEST(ReportGeneratorTest, GeneratesReportForEmptyAnalysis)
{
    const AnalysisModel model(Path("empty.log"), 0U);

    ReportGenerator generator;

    const Report report = generator.generate(model);

    EXPECT_NE(std::string::npos, report.text().find("Total log lines : 0"));
    EXPECT_NE(std::string::npos, report.text().find("Error lines     : 0"));
}
