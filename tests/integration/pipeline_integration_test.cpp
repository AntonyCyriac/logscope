/**
 * @file pipeline_integration_test.cpp
 * @brief Integration tests for the core analysis pipeline.
 */

#include <fstream>

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "configuration_manager.hpp"
#include "investigation.hpp"
#include "reporting.hpp"
#include "source.hpp"

using scope::analysis::AnalysisEngine;
using scope::configuration::ConfigurationManager;
using scope::foundation::Path;
using scope::investigation::InvestigationEngine;
using scope::investigation::LineCountFilter;
using scope::reporting::ReportGenerator;
using scope::source::SourceManager;

namespace
{

class PipelineIntegrationTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        m_testFile = Path("pipeline_integration_test.log");

        std::ofstream stream(m_testFile.string());

        stream << "2026-07-11 10:00:01 INFO Application started\n";
        stream << "2026-07-11 10:00:05 INFO Connecting to database\n";
        stream << "2026-07-11 10:00:06 ERROR Connection refused\n";
        stream << "2026-07-11 10:00:07 ERROR Connection refused\n";
        stream << "2026-07-11 10:00:10 WARNING Request taking too long\n";
        stream << "2026-07-11 10:00:15 ERROR Request timeout\n";
        stream << "2026-07-11 10:00:20 INFO Retrying request\n";
        stream << "2026-07-11 10:00:25 ERROR Connection refused\n";
    }

    void TearDown() override
    {
        std::remove(m_testFile.string().c_str());
    }

    Path m_testFile;
};

} // namespace

TEST_F(PipelineIntegrationTest, RunsSourceToReportPipeline)
{
    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(m_testFile);

    ASSERT_TRUE(datasetResult.hasValue());

    AnalysisEngine analysisEngine;

    const auto modelResult = analysisEngine.analyze(*datasetResult);

    ASSERT_TRUE(modelResult.hasValue());
    EXPECT_EQ(8U, modelResult->totalLines());

    InvestigationEngine investigationEngine;

    const auto view = investigationEngine.inspect(*modelResult);

    EXPECT_FALSE(view.isEmpty());
    EXPECT_TRUE(investigationEngine.matches(*modelResult, LineCountFilter::nonEmpty()));
    EXPECT_TRUE(investigationEngine.searchSource(*modelResult, "pipeline_integration"));

    const auto report = ReportGenerator{}.generate(*modelResult);

    EXPECT_NE(std::string::npos, report.text().find("Total log lines : 8"));
    EXPECT_NE(std::string::npos, report.text().find(m_testFile.string()));
}

TEST_F(PipelineIntegrationTest, LoadsConfigurationBeforePipeline)
{
    const Path configFile("pipeline_integration_config.properties");

    {
        std::ofstream stream(configFile.string());

        stream << "log.level=error\n";
    }

    auto configurationResult = ConfigurationManager::loadFromFile(configFile);

    ASSERT_TRUE(configurationResult.hasValue());

    configurationResult->applyEnvironment();

    const auto validationResult = configurationResult->validate({"log.level"});

    ASSERT_TRUE(validationResult.hasValue());
    EXPECT_TRUE(*validationResult);

    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(m_testFile);

    ASSERT_TRUE(datasetResult.hasValue());

    const auto modelResult = AnalysisEngine{}.analyze(*datasetResult);

    ASSERT_TRUE(modelResult.hasValue());
    EXPECT_EQ(8U, modelResult->totalLines());

    std::remove(configFile.string().c_str());
}

TEST_F(PipelineIntegrationTest, InvestigationFilterExcludesEmptyAnalysis)
{
    const Path emptyFile("pipeline_integration_empty.log");

    {
        std::ofstream stream(emptyFile.string());
    }

    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(emptyFile);

    ASSERT_TRUE(datasetResult.hasValue());

    const auto modelResult = AnalysisEngine{}.analyze(*datasetResult);

    ASSERT_TRUE(modelResult.hasValue());
    EXPECT_EQ(0U, modelResult->totalLines());

    InvestigationEngine investigationEngine;

    EXPECT_FALSE(investigationEngine.matches(*modelResult, LineCountFilter::nonEmpty()));

    const auto report = ReportGenerator{}.generate(*modelResult);

    EXPECT_NE(std::string::npos, report.text().find("Total log lines : 0"));

    std::remove(emptyFile.string().c_str());
}
