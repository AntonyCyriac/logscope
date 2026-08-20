/**
 * @file run_comparison_test.cpp
 * @brief Run comparison tests (ADR-014 / RC.1–RC.8).
 */

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "analysis.hpp"
#include "compare.hpp"
#include "gtest_temp_path.hpp"
#include "source.hpp"

using scope::analysis::AnalysisEngine;
using scope::analysis::LogFormat;
using scope::compare::ComparisonResult;
using scope::compare::IncomparableReason;
using scope::compare::compareModels;
using scope::foundation::Path;
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

[[nodiscard]] scope::analysis::AnalysisModel analyzePath(const Path& path)
{
    SourceManager manager;
    auto dataset = manager.open(path);

    EXPECT_TRUE(dataset.hasValue());

    AnalysisEngine engine;
    scope::analysis::AnalysisConfig config = scope::analysis::AnalysisConfig::defaults();
    config.formatHint = LogFormat::PlainText;

    const auto model = engine.analyze(*dataset, config);

    EXPECT_TRUE(model.hasValue());

    return std::move(*model);
}

} // namespace

TEST(RunComparisonTest, SingleFileSignatureDiff)
{
    const Path baselinePath(logscope::gtest::uniqueTestPath("_rc1_good.log"));
    const Path candidatePath(logscope::gtest::uniqueTestPath("_rc1_bad.log"));

    writeTextFile(baselinePath, "2026-01-01 ERROR healthy heartbeat ok\n");
    writeTextFile(candidatePath, "2026-01-01 ERROR timeout waiting for ack\n");

    const ComparisonResult result = compareModels(analyzePath(baselinePath), analyzePath(candidatePath));

    EXPECT_TRUE(result.comparable);
    ASSERT_EQ(1U, result.onlyInBaseline.size());
    ASSERT_EQ(1U, result.onlyInCandidate.size());
    EXPECT_NE(result.onlyInBaseline.front().signature, result.onlyInCandidate.front().signature);

    std::filesystem::remove(baselinePath.string());
    std::filesystem::remove(candidatePath.string());
}

TEST(RunComparisonTest, CountDeltaReported)
{
    const Path baselinePath(logscope::gtest::uniqueTestPath("_rc2_good.log"));
    const Path candidatePath(logscope::gtest::uniqueTestPath("_rc2_bad.log"));

    writeTextFile(baselinePath,
                  "2026-01-01 ERROR handler error\n"
                  "2026-01-01 ERROR handler error\n"
                  "2026-01-01 ERROR handler error\n"
                  "2026-01-01 ERROR handler error\n");
    writeTextFile(candidatePath,
                  "2026-01-01 ERROR handler error\n"
                  "2026-01-01 ERROR handler error\n"
                  "2026-01-01 ERROR handler error\n");

    const ComparisonResult result = compareModels(analyzePath(baselinePath), analyzePath(candidatePath));

    EXPECT_TRUE(result.comparable);
    ASSERT_FALSE(result.countDeltas.empty());
    EXPECT_EQ(4U, result.countDeltas.front().baseline);
    EXPECT_EQ(3U, result.countDeltas.front().candidate);

    std::filesystem::remove(baselinePath.string());
    std::filesystem::remove(candidatePath.string());
}

TEST(RunComparisonTest, VanishedSignatureInOnlyInBaseline)
{
    const Path baselinePath(logscope::gtest::uniqueTestPath("_rc3_good.log"));
    const Path candidatePath(logscope::gtest::uniqueTestPath("_rc3_bad.log"));

    writeTextFile(baselinePath, "2026-01-01 ERROR expected scheduler tick\n");
    writeTextFile(candidatePath, "2026-01-01 INFO all quiet\n");

    const ComparisonResult result = compareModels(analyzePath(baselinePath), analyzePath(candidatePath));

    EXPECT_TRUE(result.comparable);
    ASSERT_EQ(1U, result.onlyInBaseline.size());
    EXPECT_TRUE(result.onlyInCandidate.empty());

    std::filesystem::remove(baselinePath.string());
    std::filesystem::remove(candidatePath.string());
}

TEST(RunComparisonTest, TwoInstanceBundleCompare)
{
    const Path baselineRoot(logscope::gtest::uniqueTestPath("_rc4_base"));
    const Path candidateRoot(logscope::gtest::uniqueTestPath("_rc4_cand"));

    writeTextFile(baselineRoot.append("inst-a/app.log"), "2026-01-01 ERROR alpha failure\n");
    writeTextFile(baselineRoot.append("inst-b/app.log"), "2026-01-01 ERROR beta failure\n");
    writeTextFile(candidateRoot.append("inst-a/app.log"), "2026-01-01 ERROR alpha failure\n");
    writeTextFile(candidateRoot.append("inst-b/app.log"), "2026-01-01 ERROR beta failure changed\n");

    const ComparisonResult result = compareModels(analyzePath(baselineRoot), analyzePath(candidateRoot));

    EXPECT_TRUE(result.comparable);
    EXPECT_GE(result.perInstance.size(), 2U);

    std::filesystem::remove_all(baselineRoot.string());
    std::filesystem::remove_all(candidateRoot.string());
}

TEST(RunComparisonTest, InstanceMismatchIncomparable)
{
    const Path baselineRoot(logscope::gtest::uniqueTestPath("_rc5_base"));
    const Path candidateRoot(logscope::gtest::uniqueTestPath("_rc5_cand"));

    writeTextFile(baselineRoot.append("inst-a/app.log"), "2026-01-01 ERROR alpha failure\n");
    writeTextFile(candidateRoot.append("inst-z/app.log"), "2026-01-01 ERROR zulu failure\n");

    const ComparisonResult result = compareModels(analyzePath(baselineRoot), analyzePath(candidateRoot));

    EXPECT_FALSE(result.comparable);
    ASSERT_TRUE(result.incomparableReason.has_value());
    EXPECT_EQ(IncomparableReason::NoInstanceOverlap, *result.incomparableReason);

    std::filesystem::remove_all(baselineRoot.string());
    std::filesystem::remove_all(candidateRoot.string());
}

TEST(RunComparisonTest, NoFileOverlapIncomparable)
{
    const Path baselineRoot(logscope::gtest::uniqueTestPath("_rc6_base"));
    const Path candidateRoot(logscope::gtest::uniqueTestPath("_rc6_cand"));

    writeTextFile(baselineRoot.append("a.log"), "2026-01-01 ERROR alpha failure\n");
    writeTextFile(candidateRoot.append("z.log"), "2026-01-01 ERROR zulu failure\n");

    const ComparisonResult result = compareModels(analyzePath(baselineRoot), analyzePath(candidateRoot));

    EXPECT_FALSE(result.comparable);
    ASSERT_TRUE(result.incomparableReason.has_value());
    EXPECT_EQ(IncomparableReason::NoFileOverlap, *result.incomparableReason);

    std::filesystem::remove_all(baselineRoot.string());
    std::filesystem::remove_all(candidateRoot.string());
}

TEST(RunComparisonTest, PartialSkipBoundedCompare)
{
    const Path baselineRoot(logscope::gtest::uniqueTestPath("_rc7_base"));
    const Path candidateRoot(logscope::gtest::uniqueTestPath("_rc7_cand"));

    writeTextFile(baselineRoot.append("good.log"), "2026-01-01 ERROR baseline error\n");
    writeTextFile(candidateRoot.append("good.log"), "2026-01-01 ERROR candidate error\n");
    writeBinaryFile(candidateRoot.append("core.dump"));

    const ComparisonResult result = compareModels(analyzePath(baselineRoot), analyzePath(candidateRoot));

    EXPECT_TRUE(result.comparable);
    EXPECT_FALSE(result.complete);
    ASSERT_FALSE(result.warnings.empty());
    EXPECT_EQ("COMPARISON_BOUNDED", result.warnings.front().code);

    std::filesystem::remove_all(baselineRoot.string());
    std::filesystem::remove_all(candidateRoot.string());
}

TEST(RunComparisonTest, IndeterminateBaselineRefused)
{
    const Path baselineRoot(logscope::gtest::uniqueTestPath("_rc8_base"));
    const Path candidatePath(logscope::gtest::uniqueTestPath("_rc8_cand.log"));

    writeTextFile(candidatePath, "2026-01-01 ERROR candidate error\n");

    scope::source::DiscoveryCensus census;
    census.root = baselineRoot;
    census.candidatesFound = 1U;

    scope::source::AnalysisAccounting accounting;
    accounting.analyzedLineCount = 0U;
    accounting.complete = false;

    const scope::analysis::AnalysisModel baselineModel(baselineRoot,
                                                       0U,
                                                       {},
                                                       LogFormat::PlainText,
                                                       std::nullopt,
                                                       std::nullopt,
                                                       std::nullopt,
                                                       nullptr,
                                                       census,
                                                       accounting);

    const ComparisonResult result = compareModels(baselineModel, analyzePath(candidatePath));

    EXPECT_FALSE(result.comparable);
    ASSERT_TRUE(result.incomparableReason.has_value());
    EXPECT_EQ(IncomparableReason::BaselineIndeterminate, *result.incomparableReason);

    std::filesystem::remove(candidatePath.string());
}
