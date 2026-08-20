/**
 * @file rotation_grouper_test.cpp
 * @brief Rotation stream grouping tests (ADR-013 / II.2).
 */

#include <gtest/gtest.h>

#include "foundation/path.hpp"
#include "rotation_grouper.hpp"

using scope::foundation::Path;
using scope::source::IngestFile;
using scope::source::buildIngestStreams;
using scope::source::rotationGroupIdForPath;

TEST(RotationGrouperTest, OrdersHistoryBeforeLiveTail)
{
    EXPECT_EQ("service.log", rotationGroupIdForPath("proxy/service.log"));
    EXPECT_EQ("service.log", rotationGroupIdForPath("proxy/service.log.001"));

    std::vector<IngestFile> candidates;

    candidates.push_back(IngestFile{"proxy/service.log", Path("proxy/service.log"), "default"});
    candidates.push_back(IngestFile{"proxy/service.log.003", Path("proxy/service.log.003"), "default"});
    candidates.push_back(IngestFile{"proxy/service.log.001", Path("proxy/service.log.001"), "default"});
    candidates.push_back(IngestFile{"proxy/service.log.002", Path("proxy/service.log.002"), "default"});

    const std::vector<scope::source::IngestStream> streams = buildIngestStreams(Path("proxy"), candidates);

    ASSERT_EQ(1U, streams.size());
    ASSERT_EQ(4U, streams.front().orderedFiles.size());
    EXPECT_EQ("proxy/service.log.001", streams.front().orderedFiles[0].relativePath);
    EXPECT_EQ("proxy/service.log.002", streams.front().orderedFiles[1].relativePath);
    EXPECT_EQ("proxy/service.log.003", streams.front().orderedFiles[2].relativePath);
    EXPECT_EQ("proxy/service.log", streams.front().orderedFiles[3].relativePath);
}
