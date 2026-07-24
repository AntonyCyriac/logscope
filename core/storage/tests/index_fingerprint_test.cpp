/**
 * @file index_fingerprint_test.cpp
 */

#include <gtest/gtest.h>

#include <fstream>

#include "index_fingerprint.hpp"

using scope::foundation::Path;
using scope::storage::IndexFingerprint;

TEST(IndexFingerprintTest, ComputesStableFingerprint)
{
    const Path tempFile("storage_fingerprint_test.log");
    std::ofstream output(tempFile.string());
    output << "sample line\n";
    output.close();

    const auto fingerprint = IndexFingerprint::compute(tempFile);

    ASSERT_TRUE(fingerprint);
    EXPECT_TRUE(IndexFingerprint::matchesSource(*fingerprint, tempFile));
}

TEST(IndexFingerprintTest, RestoresStoredFingerprint)
{
    const Path tempFile("storage_fingerprint_restore.log");
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
    const Path tempFile("storage_fingerprint_change.log");
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
