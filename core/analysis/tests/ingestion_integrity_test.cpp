/**
 * @file ingestion_integrity_test.cpp
 * @brief End-to-end ingestion integrity tests (ADR-013 / II.2, II.4–II.6, II.8).
 */

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "gtest_temp_path.hpp"
#include "source.hpp"

using scope::analysis::AnalysisEngine;
using scope::analysis::LogFormat;
using scope::foundation::Path;
using scope::source::CandidateDisposition;
using scope::source::SourceManager;

namespace
{

void writeTextFile(const Path& path, const std::string& content)
{
    const std::filesystem::path parent = std::filesystem::path(path.string()).parent_path();

    if (!parent.empty())
    {
        std::filesystem::create_directories(parent);
    }

    std::ofstream stream(path.string());
    stream << content;
}

void writeBinaryFile(const Path& path)
{
    std::ofstream stream(path.string(), std::ios::binary);

    for (int index = 0; index < 256; ++index)
    {
        stream.put(static_cast<char>(index % 32));
    }
}

} // namespace

TEST(IngestionIntegrityTest, RotationStreamFullHistory)
{
    const Path root(logscope::gtest::uniqueTestPath("_rotation_root"));

    writeTextFile(root.append("service.log"), "live\nlive\n");
    writeTextFile(root.append("service.log.001"), "one\none\ntwo\n");
    writeTextFile(root.append("service.log.002"), "hist\n");

    SourceManager manager;
    auto dataset = manager.open(root);

    ASSERT_TRUE(dataset.hasValue());

    AnalysisEngine engine;
    scope::analysis::AnalysisConfig config = scope::analysis::AnalysisConfig::defaults();
    config.formatHint = LogFormat::PlainText;

    const auto model = engine.analyze(*dataset, config);

    ASSERT_TRUE(model.hasValue());
    EXPECT_EQ(6U, model->totalLines());
    ASSERT_TRUE(model->discoveryCensus().has_value());
    EXPECT_FALSE(model->discoveryCensus()->rotationGroups.empty());

    std::filesystem::remove_all(root.string());
}

TEST(IngestionIntegrityTest, MultiFileIdentityOnMatch)
{
    const Path root(logscope::gtest::uniqueTestPath("_identity_root"));

    writeTextFile(root.append("a.log"), "2026-01-01 ERROR first in a\n");
    writeTextFile(root.append("b.log"), "2026-01-01 ERROR first in b\n");

    SourceManager manager;
    auto dataset = manager.open(root);

    ASSERT_TRUE(dataset.hasValue());

    AnalysisEngine engine;
    scope::analysis::AnalysisConfig config = scope::analysis::AnalysisConfig::defaults();
    config.formatHint = LogFormat::PlainText;

    const auto model = engine.analyze(*dataset, config);

    ASSERT_TRUE(model.hasValue());
    ASSERT_TRUE(model->lineIndex().has_value());

    const std::vector<scope::analysis::IndexedLine>& lines = model->lineIndex()->lines();
    ASSERT_EQ(2U, lines.size());
    EXPECT_EQ("a.log", lines[0].sourceFileRelative);
    EXPECT_EQ(1U, lines[0].fileLineNumber);
    EXPECT_EQ("b.log", lines[1].sourceFileRelative);
    EXPECT_EQ(1U, lines[1].fileLineNumber);
    EXPECT_NE(lines[0].lineNumber, lines[1].lineNumber);

    std::filesystem::remove_all(root.string());
}

TEST(IngestionIntegrityTest, TwoInstanceBundleGrouping)
{
    const Path root(logscope::gtest::uniqueTestPath("_instances_root"));

    writeTextFile(root.append("inst-a").append("service.log"), "alpha\n");
    writeTextFile(root.append("inst-b").append("service.log"), "beta\n");

    SourceManager manager;
    auto dataset = manager.open(root);

    ASSERT_TRUE(dataset.hasValue());

    AnalysisEngine engine;

    const auto model = engine.analyze(*dataset);

    ASSERT_TRUE(model.hasValue());
    ASSERT_TRUE(model->discoveryCensus().has_value());
    EXPECT_EQ(2U, model->discoveryCensus()->instances.size());

    std::filesystem::remove_all(root.string());
}

TEST(IngestionIntegrityTest, UnknownDialectReported)
{
    const Path file(logscope::gtest::uniqueTestPath("_dialect.log"));

    writeTextFile(file, "19-08-2026 10:00:00 status update\n");

    SourceManager manager;
    auto dataset = manager.open(file);

    ASSERT_TRUE(dataset.hasValue());

    AnalysisEngine engine;
    scope::analysis::AnalysisConfig config = scope::analysis::AnalysisConfig::defaults();
    config.formatHint = LogFormat::PlainText;

    const auto model = engine.analyze(*dataset, config);

    ASSERT_TRUE(model.hasValue());
    ASSERT_TRUE(model->analysisAccounting().has_value());
    EXPECT_FALSE(model->analysisAccounting()->complete);

    bool foundWarning = false;

    for (const scope::source::IngestionWarning& warning : model->analysisAccounting()->warnings)
    {
        if (warning.code == "UNKNOWN_TIMESTAMP_DIALECT")
        {
            foundWarning = true;
        }
    }

    EXPECT_TRUE(foundWarning);

    std::remove(file.string().c_str());
}

TEST(IngestionIntegrityTest, PartialSkipIncompleteFlag)
{
    const Path root(logscope::gtest::uniqueTestPath("_partial_skip"));

    writeTextFile(root.append("good.log"), "2026-01-01 INFO ok\n");
    writeBinaryFile(root.append("bad.bin"));

    SourceManager manager;
    auto dataset = manager.open(root);

    ASSERT_TRUE(dataset.hasValue());

    AnalysisEngine engine;
    scope::analysis::AnalysisConfig config = scope::analysis::AnalysisConfig::defaults();
    config.formatHint = LogFormat::PlainText;

    const auto model = engine.analyze(*dataset, config);

    ASSERT_TRUE(model.hasValue());
    ASSERT_TRUE(model->discoveryCensus().has_value());
    ASSERT_TRUE(model->analysisAccounting().has_value());
    EXPECT_GT(model->discoveryCensus()->candidatesFound, model->discoveryCensus()->analyzedCount);
    EXPECT_FALSE(model->analysisAccounting()->complete);

    bool skippedBinary = false;

    for (const scope::source::DiscoveryEntry& entry : model->discoveryCensus()->entries)
    {
        if (entry.disposition == CandidateDisposition::Skipped)
        {
            skippedBinary = true;
        }
    }

    EXPECT_TRUE(skippedBinary);

    std::filesystem::remove_all(root.string());
}
