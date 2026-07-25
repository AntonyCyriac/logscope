/**
 * @file index_fingerprint_test.cpp
 */

#include <gtest/gtest.h>

#include <fstream>

#include "gtest_temp_path.hpp"
#include "index_fingerprint.hpp"

using scope::foundation::Path;
using scope::storage::IndexFingerprint;

TEST(IndexFingerprintTest, ComputesStableFingerprint)
{
    const Path tempFile(logscope::gtest::uniqueTestPath("_stable.log"));
    std::ofstream output(tempFile.string());
    output << "sample line\n";
    output.close();

    const auto fingerprint = IndexFingerprint::compute(tempFile);

    ASSERT_TRUE(fingerprint);
    EXPECT_TRUE(IndexFingerprint::matchesSource(*fingerprint, tempFile));
}

TEST(IndexFingerprintTest, RestoresStoredFingerprint)
{
    const Path tempFile(logscope::gtest::uniqueTestPath("_restore.log"));
    std::ofstream output(tempFile.string());
    output << "restore me\n";
    output.close();

    const auto fingerprint = IndexFingerprint::compute(tempFile);
    ASSERT_TRUE(fingerprint);

    const auto restored = IndexFingerprint::fromStored(fingerprint->value());

    EXPECT_EQ(fingerprint->value(), restored.value());
}

TEST(IndexFingerprintTest, DetectsChangedSource)
{
    const Path tempFile(logscope::gtest::uniqueTestPath("_change.log"));
    std::ofstream output(tempFile.string());
    output << "first\n";
    output.close();

    const auto fingerprint = IndexFingerprint::compute(tempFile);
    ASSERT_TRUE(fingerprint);

    std::ofstream append(tempFile.string(), std::ios::app);
    append << "second\n";
    append.close();

    const auto matches = IndexFingerprint::matchesSource(*fingerprint, tempFile);

    ASSERT_TRUE(matches);
    EXPECT_FALSE(*matches);
}

TEST(IndexFingerprintTest, ComputesStablePathKeyIndependentOfContent)
{
    const Path firstFile(logscope::gtest::uniqueTestPath("_path_first.log"));
    const Path secondFile(logscope::gtest::uniqueTestPath("_path_second.log"));

    std::ofstream firstOutput(firstFile.string());
    firstOutput << "alpha\n";
    firstOutput.close();

    std::ofstream secondOutput(secondFile.string());
    secondOutput << "beta\n";
    secondOutput.close();

    const auto firstKey = IndexFingerprint::stablePathKey(firstFile);
    const auto secondKey = IndexFingerprint::stablePathKey(secondFile);
    const auto firstFingerprint = IndexFingerprint::compute(firstFile);

    ASSERT_TRUE(firstKey);
    ASSERT_TRUE(secondKey);
    ASSERT_TRUE(firstFingerprint);

    EXPECT_NE(firstKey->value(), secondKey->value());
    EXPECT_NE(firstKey->value(), firstFingerprint->value());
}
