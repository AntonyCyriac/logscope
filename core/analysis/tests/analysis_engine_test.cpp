/**
 * @file analysis_engine_test.cpp
 * @brief Unit tests for AnalysisEngine.
 */

#include <fstream>

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "source.hpp"

using scope::analysis::AnalysisEngine;
using scope::foundation::Path;
using scope::source::SourceDataset;
using scope::source::SourceManager;

namespace
{

class AnalysisEngineTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        m_testFile = Path("analysis_engine_test.log");

        std::ofstream stream(m_testFile.string());

        stream << "line one\n";
        stream << "line two\n";
        stream << "line three\n";
    }

    void writeLevelTestFile()
    {
        m_levelTestFile = Path("analysis_engine_levels_test.log");

        std::ofstream stream(m_levelTestFile.string());

        stream << "2026-07-11 10:00:01 INFO Application started\n";
        stream << "2026-07-11 10:00:10 WARNING Request taking too long\n";
        stream << "2026-07-11 10:00:06 ERROR Connection refused\n";
        stream << "2026-07-11 10:00:07 ERROR Connection refused\n";
    }

    void TearDown() override
    {
        std::remove(m_testFile.string().c_str());
        std::remove(m_levelTestFile.string().c_str());
    }

    Path m_testFile;
    Path m_levelTestFile;
};

} // namespace

TEST_F(AnalysisEngineTest, AnalyzesSourceDataset)
{
    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(m_testFile);

    ASSERT_TRUE(datasetResult.hasValue());

    AnalysisEngine engine;

    const auto modelResult = engine.analyze(*datasetResult);

    ASSERT_TRUE(modelResult.hasValue());
    EXPECT_EQ(m_testFile.string(), modelResult->sourcePath().string());
    EXPECT_EQ(3U, modelResult->totalLines());
}

TEST_F(AnalysisEngineTest, AnalyzesEmptyFile)
{
    const Path emptyFile("analysis_engine_empty_test.log");

    {
        std::ofstream stream(emptyFile.string());
    }

    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(emptyFile);

    ASSERT_TRUE(datasetResult.hasValue());

    AnalysisEngine engine;

    const auto modelResult = engine.analyze(*datasetResult);

    ASSERT_TRUE(modelResult.hasValue());
    EXPECT_EQ(0U, modelResult->totalLines());

    std::remove(emptyFile.string().c_str());
}

TEST_F(AnalysisEngineTest, RejectsInvalidDataset)
{
    AnalysisEngine engine;

    SourceDataset dataset;
    const auto modelResult = engine.analyze(dataset);

    ASSERT_TRUE(modelResult.hasError());
    EXPECT_EQ(scope::foundation::ErrorCode::InvalidArgument, modelResult.error().code());
}

TEST_F(AnalysisEngineTest, CountsLogLevels)
{
    writeLevelTestFile();

    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(m_levelTestFile);

    ASSERT_TRUE(datasetResult.hasValue());

    const auto modelResult = AnalysisEngine{}.analyze(*datasetResult);

    ASSERT_TRUE(modelResult.hasValue());
    EXPECT_EQ(4U, modelResult->totalLines());
    EXPECT_EQ(1U, modelResult->levelCounts().infoLines());
    EXPECT_EQ(1U, modelResult->levelCounts().warnLines());
    EXPECT_EQ(2U, modelResult->levelCounts().errorLines());
}

TEST_F(AnalysisEngineTest, DetectsPlainFormat)
{
    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(m_testFile);

    ASSERT_TRUE(datasetResult.hasValue());

    const auto modelResult = AnalysisEngine{}.analyze(*datasetResult);

    ASSERT_TRUE(modelResult.hasValue());
    EXPECT_EQ(scope::analysis::LogFormat::PlainText, modelResult->format());
}

TEST_F(AnalysisEngineTest, RejectsBinaryInput)
{
    const Path binaryFile("analysis_engine_binary_test.bin");

    {
        std::ofstream stream(binaryFile.string(), std::ios::binary);
        stream.write("abc\0def\ngh", 10);
        stream.put('\0');
        stream << "more";
    }

    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(binaryFile);

    ASSERT_TRUE(datasetResult.hasValue());

    const auto modelResult = AnalysisEngine{}.analyze(*datasetResult);

    ASSERT_TRUE(modelResult.hasError());
    EXPECT_EQ(scope::foundation::ErrorCode::InvalidArgument, modelResult.error().code());
    EXPECT_NE(std::string::npos, modelResult.error().message().find("binary"));

    std::remove(binaryFile.string().c_str());
}

TEST_F(AnalysisEngineTest, HonorsPlainFormatOverride)
{
    writeLevelTestFile();

    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(m_levelTestFile);

    ASSERT_TRUE(datasetResult.hasValue());

    const auto modelResult =
        AnalysisEngine{}.analyze(*datasetResult, scope::analysis::LogFormat::PlainText);

    ASSERT_TRUE(modelResult.hasValue());
    EXPECT_EQ(scope::analysis::LogFormat::PlainText, modelResult->format());
}

TEST_F(AnalysisEngineTest, AnalyzesJsonLinesWithFieldAwareLevels)
{
    const Path jsonFile("analysis_engine_json_test.jsonl");

    {
        std::ofstream stream(jsonFile.string());

        stream << R"({"level":"info","message":"started"})" << '\n';
        stream << R"({"severity":"warning","message":"slow"})" << '\n';
        stream << R"({"level":"error","message":"failed"})" << '\n';
        stream << R"({"level":"error","message":"failed again"})" << '\n';
    }

    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(jsonFile);

    ASSERT_TRUE(datasetResult.hasValue());

    const auto modelResult = AnalysisEngine{}.analyze(*datasetResult);

    ASSERT_TRUE(modelResult.hasValue());
    EXPECT_EQ(scope::analysis::LogFormat::JsonLines, modelResult->format());
    EXPECT_EQ(4U, modelResult->totalLines());
    EXPECT_EQ(1U, modelResult->levelCounts().infoLines());
    EXPECT_EQ(1U, modelResult->levelCounts().warnLines());
    EXPECT_EQ(2U, modelResult->levelCounts().errorLines());
    ASSERT_TRUE(modelResult->jsonLinesSummary().has_value());
    EXPECT_EQ(4U, modelResult->jsonLinesSummary()->validLines());
    EXPECT_EQ(0U, modelResult->jsonLinesSummary()->parseFailures());

    std::remove(jsonFile.string().c_str());
}

TEST_F(AnalysisEngineTest, ContinuesAfterMalformedJsonLines)
{
    const Path jsonFile("analysis_engine_json_mixed_test.jsonl");

    {
        std::ofstream stream(jsonFile.string());

        stream << R"({"level":"info","message":"ok"})" << '\n';
        stream << "not json" << '\n';
        stream << R"({"level":"error","message":"bad"})" << '\n';
    }

    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(jsonFile);

    ASSERT_TRUE(datasetResult.hasValue());

    const auto modelResult =
        AnalysisEngine{}.analyze(*datasetResult, scope::analysis::LogFormat::JsonLines);

    ASSERT_TRUE(modelResult.hasValue());
    EXPECT_EQ(3U, modelResult->totalLines());
    ASSERT_TRUE(modelResult->jsonLinesSummary().has_value());
    EXPECT_EQ(2U, modelResult->jsonLinesSummary()->validLines());
    EXPECT_EQ(1U, modelResult->jsonLinesSummary()->parseFailures());
    EXPECT_EQ(1U, modelResult->levelCounts().infoLines());
    EXPECT_EQ(1U, modelResult->levelCounts().errorLines());
    EXPECT_EQ(1U, modelResult->levelCounts().otherLines());

    std::remove(jsonFile.string().c_str());
}

TEST_F(AnalysisEngineTest, ExtractsPlainTextTimeRange)
{
    writeLevelTestFile();

    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(m_levelTestFile);

    ASSERT_TRUE(datasetResult.hasValue());

    const auto modelResult = AnalysisEngine{}.analyze(*datasetResult);

    ASSERT_TRUE(modelResult.hasValue());
    ASSERT_TRUE(modelResult->fieldSummary().has_value());
    EXPECT_GT(modelResult->fieldSummary()->linesWithTimestamp(), 0U);
    EXPECT_TRUE(modelResult->fieldSummary()->earliestTimestamp().has_value());
    EXPECT_TRUE(modelResult->fieldSummary()->latestTimestamp().has_value());
    EXPECT_GT(modelResult->fieldSummary()->linesWithMessage(), 0U);
}

TEST_F(AnalysisEngineTest, ExtractsJsonLinesTimeRange)
{
    const Path jsonFile("analysis_engine_json_fields_test.jsonl");

    {
        std::ofstream stream(jsonFile.string());

        stream << R"({"timestamp":"2026-07-21T10:00:01Z","message":"started","level":"info"})" << '\n';
        stream << R"({"timestamp":"2026-07-21T10:00:25Z","message":"failed","level":"error"})" << '\n';
    }

    SourceManager sourceManager;

    auto datasetResult = sourceManager.open(jsonFile);

    ASSERT_TRUE(datasetResult.hasValue());

    const auto modelResult =
        AnalysisEngine{}.analyze(*datasetResult, scope::analysis::LogFormat::JsonLines);

    ASSERT_TRUE(modelResult.hasValue());
    ASSERT_TRUE(modelResult->fieldSummary().has_value());
    EXPECT_EQ(2U, modelResult->fieldSummary()->linesWithTimestamp());
    EXPECT_TRUE(modelResult->fieldSummary()->earliestTimestamp().has_value());
    EXPECT_TRUE(modelResult->fieldSummary()->latestTimestamp().has_value());
    EXPECT_EQ(2U, modelResult->fieldSummary()->linesWithMessage());

    std::remove(jsonFile.string().c_str());
}
