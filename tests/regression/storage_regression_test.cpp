/**
 * @file storage_regression_test.cpp
 * @brief Regression guards for M11 storage integration edge cases.
 */

#include <fstream>
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

#include "analysis.hpp"
#include "source.hpp"

using scope::analysis::AnalysisConfig;
using scope::analysis::AnalysisEngine;
using scope::foundation::Path;
using scope::source::SourceManager;

namespace
{

class StorageRegressionTest : public ::testing::Test
{
  protected:
    void TearDown() override
    {
        std::error_code error;
        std::filesystem::remove_all(m_directoryPath.string(), error);

        if (!m_logFilePath.isEmpty())
        {
            std::filesystem::remove(m_logFilePath.string(), error);
            m_logFilePath = Path{};
        }
    }

    Path m_directoryPath{"regression_log_directory"};
    Path m_logFilePath;
};

} // namespace

// M11 v1.4.1: fingerprint compute on stdin broke analyze when reuse-index was set.
TEST_F(StorageRegressionTest, AnalyzeStdinWithReuseIndexDoesNotFail)
{
    std::istringstream input("2026-07-11 10:00:01 INFO stdin line\n"
                              "2026-07-11 10:00:02 ERROR stdin error\n");

    std::streambuf* const previousInput = std::cin.rdbuf(input.rdbuf());

    SourceManager sourceManager;
    auto datasetResult = sourceManager.open(Path("-"));

    ASSERT_TRUE(datasetResult.hasValue());

    AnalysisConfig config = AnalysisConfig::defaults();
    config.storage.reuseIndex = true;

    const AnalysisEngine engine;
    const auto result = engine.analyze(*datasetResult, config);

    std::cin.rdbuf(previousInput);

    ASSERT_TRUE(result.hasValue()) << result.error().message();
    EXPECT_EQ(2U, result->totalLines());
    EXPECT_EQ("-", result->sourcePath().string());
}

// M11 v1.4.1: fingerprint on non-regular paths (directories) must not break analysis.
TEST_F(StorageRegressionTest, AnalyzeDirectoryWithPersistIndexDoesNotFail)
{
    std::filesystem::create_directory(m_directoryPath.string());

    const Path logFile = m_directoryPath.append("app.log");
    std::ofstream output(logFile.string());
    output << "2026-07-11 10:00:01 INFO directory source\n";
    output << "2026-07-11 10:00:02 ERROR directory error\n";
    output.close();

    {
        SourceManager sourceManager;
        auto datasetResult = sourceManager.open(m_directoryPath);

        ASSERT_TRUE(datasetResult.hasValue());

        AnalysisConfig config = AnalysisConfig::defaults();
        config.storage.persistIndex = true;

        const AnalysisEngine engine;
        const auto result = engine.analyze(*datasetResult, config);

        ASSERT_TRUE(result.hasValue()) << result.error().message();
        EXPECT_EQ(2U, result->totalLines());
    }
}

// M11 v1.4.1: analyze on a regular file with reuse-index enabled must remain stable.
TEST_F(StorageRegressionTest, AnalyzeFileWithReuseIndexSucceeds)
{
    m_logFilePath = Path("regression_reuse_index.log");

    {
        std::ofstream output(m_logFilePath.string());
        output << "2026-07-11 10:00:01 INFO reuse test\n";
    }

    {
        SourceManager sourceManager;
        auto datasetResult = sourceManager.open(m_logFilePath);

        ASSERT_TRUE(datasetResult.hasValue());

        AnalysisConfig config = AnalysisConfig::defaults();
        config.storage.reuseIndex = true;

        const AnalysisEngine engine;
        const auto result = engine.analyze(*datasetResult, config);

        ASSERT_TRUE(result.hasValue()) << result.error().message();
        EXPECT_EQ(1U, result->totalLines());
    }
}
