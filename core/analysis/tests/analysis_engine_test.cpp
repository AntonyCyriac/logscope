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

    void TearDown() override
    {
        std::remove(m_testFile.string().c_str());
    }

    Path m_testFile;
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
