/**
 * @file pipeline_integration_test.cpp
 * @brief Integration tests for the core analysis pipeline.
 */

#include <fstream>

#include <filesystem>
#include <gtest/gtest.h>

#include "analysis.hpp"
#include "configuration_manager.hpp"
#include "extension.hpp"
#include "investigation.hpp"
#include "reporting.hpp"
#include "source.hpp"
#include "workspace.hpp"
#include "search_history.hpp"

using scope::analysis::AnalysisEngine;
using scope::analysis::AnalysisModel;
using scope::analysis::LogLevelCounts;
using scope::configuration::ConfigurationManager;
using scope::foundation::Path;
using scope::investigation::InvestigationCriteria;
using scope::search::SearchHistory;
using scope::investigation::InvestigationEngine;
using scope::investigation::LineCountFilter;
using scope::investigation::LogLevelFilter;
using scope::reporting::ReportGenerator;
using scope::reporting::ReportOptions;
using scope::source::SourceManager;
using scope::workspace::InvestigationSession;
using scope::workspace::SessionStore;

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
    EXPECT_EQ(4U, modelResult->levelCounts().errorLines());
    EXPECT_TRUE(investigationEngine.matches(*modelResult, LineCountFilter::nonEmpty()));
    EXPECT_TRUE(investigationEngine.matches(*modelResult, LogLevelFilter::any().withMinimumErrors(1U)));
    EXPECT_TRUE(investigationEngine.searchSource(*modelResult, "pipeline_integration"));

    const auto report = ReportGenerator{}.generate(*modelResult);

    EXPECT_NE(std::string::npos, report.text().find("Total log lines : 8"));
    EXPECT_NE(std::string::npos, report.text().find("Error lines     : 4"));
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

TEST_F(PipelineIntegrationTest, AnalyzesDirectoryOfLogFiles)
{
    const Path directoryPath("pipeline_integration_dir");

    std::filesystem::create_directory(directoryPath.string());

    const Path firstFile = directoryPath.append("one.log");
    const Path secondFile = directoryPath.append("two.log");

    {
        std::ofstream stream(firstFile.string());
        stream << "2026-07-11 10:00:01 INFO First file\n";
        stream << "2026-07-11 10:00:02 ERROR First error\n";
    }

    {
        std::ofstream stream(secondFile.string());
        stream << "2026-07-11 10:00:03 WARNING Second file\n";
    }

    SourceManager sourceManager;

    {
        auto datasetResult = sourceManager.open(directoryPath);

        ASSERT_TRUE(datasetResult.hasValue());
        EXPECT_EQ(directoryPath.string(), datasetResult->path().string());

        const auto modelResult = AnalysisEngine{}.analyze(*datasetResult);

        ASSERT_TRUE(modelResult.hasValue());
        EXPECT_EQ(3U, modelResult->totalLines());
        EXPECT_EQ(1U, modelResult->levelCounts().errorLines());
        EXPECT_EQ(1U, modelResult->levelCounts().warnLines());

        const auto report = ReportGenerator{}.generate(*modelResult);

        EXPECT_NE(std::string::npos, report.text().find(directoryPath.string()));
    }

    std::remove(firstFile.string().c_str());
    std::remove(secondFile.string().c_str());
    std::filesystem::remove(directoryPath.string());
}

TEST_F(PipelineIntegrationTest, AppliesExtensionConfiguration)
{
    const Path configFile("pipeline_integration_extensions.properties");

    {
        std::ofstream stream(configFile.string());

        stream << "extensions.reporting.multi-format.enabled=false\n";
    }

    scope::extension::ExtensionManager manager = scope::extension::ExtensionManager::createWithBuiltIns();

    ConfigurationManager configurationManager;
    auto configurationResult = ConfigurationManager::loadFromFile(configFile);

    ASSERT_TRUE(configurationResult.hasValue());

    manager.applyConfiguration(configurationResult->configuration());

    const auto infoResult = manager.describe("reporting.multi-format");

    ASSERT_TRUE(infoResult.hasValue());
    EXPECT_FALSE(infoResult->enabled);

    std::remove(configFile.string().c_str());
}

TEST_F(PipelineIntegrationTest, SessionRoundTripPreservesInvestigationState)
{
    const Path sessionFile("pipeline_integration_session.logscope-session");
    const LogLevelCounts levelCounts = LogLevelCounts::fromCounts(3U, 1U, 4U, 0U, 0U);
    const AnalysisModel model(m_testFile, 8U, levelCounts);

    const InvestigationSession session = InvestigationSession::fromAnalysis(
        model, LineCountFilter::nonEmpty(), LogLevelFilter::any().withMinimumErrors(1U), "pipeline",
        InvestigationCriteria{}, SearchHistory{}, ReportOptions{}, Path());

    SessionStore store;

    ASSERT_TRUE(store.save(session, sessionFile).hasValue());

    const auto loaded = store.load(sessionFile);

    ASSERT_TRUE(loaded.hasValue());
    EXPECT_EQ(8U, loaded->analysisModel().totalLines());
    EXPECT_EQ(1U, loaded->levelFilter().minimumErrors());
    EXPECT_EQ("pipeline", loaded->searchQuery());

    InvestigationEngine investigationEngine;

    EXPECT_TRUE(investigationEngine.matches(loaded->analysisModel(), loaded->lineFilter()));
    EXPECT_TRUE(investigationEngine.matches(loaded->analysisModel(), loaded->levelFilter()));
    EXPECT_TRUE(investigationEngine.searchSource(loaded->analysisModel(), loaded->searchQuery()));

    std::remove(sessionFile.string().c_str());
}

TEST_F(PipelineIntegrationTest, RejectsCorruptSessionFile)
{
    const Path sessionFile("pipeline_integration_corrupt.logscope-session");

    {
        std::ofstream stream(sessionFile.string());
        stream << "source.path=sample.log\n";
        stream << "analysis.totalLines=not-a-number\n";
    }

    SessionStore store;
    const auto loaded = store.load(sessionFile);

    EXPECT_FALSE(loaded.hasValue());

    std::remove(sessionFile.string().c_str());
}

TEST_F(PipelineIntegrationTest, RejectsCorruptConfigurationFile)
{
    const Path configFile("pipeline_integration_corrupt.properties");

    {
        std::ofstream stream(configFile.string());
        stream << "invalid line without equals\n";
    }

    const auto configurationResult = ConfigurationManager::loadFromFile(configFile);

    EXPECT_FALSE(configurationResult.hasValue());

    std::remove(configFile.string().c_str());
}
