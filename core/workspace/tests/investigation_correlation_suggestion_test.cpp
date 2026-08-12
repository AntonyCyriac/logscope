/**
 * @file investigation_correlation_suggestion_test.cpp
 * @brief Unit tests for correlation suggestions (Story 6 / v2.8.0).
 */

#include <fstream>
#include <filesystem>

#include <gtest/gtest.h>

#include "gtest_temp_path.hpp"
#include "workspace.hpp"

using scope::foundation::ErrorCode;
using scope::foundation::Path;
using scope::workspace::ArtifactIngestRequest;
using scope::workspace::ArtifactSource;
using scope::workspace::CorrelationEngine;
using scope::workspace::CorrelationKey;
using scope::workspace::CorrelationSuggestionQuery;
using scope::workspace::EvidenceLinkCreateRequest;
using scope::workspace::EvidenceLinkType;
using scope::workspace::Investigation;
using scope::workspace::InvestigationCreateRequest;
using scope::workspace::TimelineEvent;

namespace
{

void writeFile(const Path& path, const std::string& contents)
{
    std::ofstream stream(path.string(), std::ios::binary | std::ios::trunc);
    stream << contents;
}

void removeDirectoryTree(const Path& directory)
{
    std::error_code errorCode;
    std::filesystem::remove_all(directory.string(), errorCode);
}

Investigation makeInvestigationWithLogs(const Path& investigationDir, const Path& appLog, const Path& syslog)
{
    InvestigationCreateRequest createRequest;
    createRequest.name = "correlation-test";

    const auto createResult = Investigation::create(investigationDir, createRequest);
    EXPECT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest appRequest;
    appRequest.type = "log";
    appRequest.name = "app.log";
    appRequest.sourceFile = appLog;
    appRequest.source = ArtifactSource{"upload", "app.log"};
    EXPECT_TRUE(investigation.addArtifact(appRequest));

    ArtifactIngestRequest syslogRequest;
    syslogRequest.type = "log";
    syslogRequest.name = "syslog";
    syslogRequest.sourceFile = syslog;
    syslogRequest.source = ArtifactSource{"upload", "syslog"};
    EXPECT_TRUE(investigation.addArtifact(syslogRequest));

    return investigation;
}

} // namespace

TEST(CorrelationEngineTest, RegexExtractsRequestIdAcrossArtifacts)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_corr_regex"));
    const Path appLog(logscope::gtest::uniqueTestPath("_app_corr.log"));
    const Path syslog(logscope::gtest::uniqueTestPath("_sys_corr.log"));

    writeFile(appLog, "2026-08-06T10:00:00 ERROR request_id=abc-123 connection failed\n");
    writeFile(syslog, "2026-08-06T10:00:01 WARNING request_id=abc-123 peer warning\n");

    Investigation investigation = makeInvestigationWithLogs(investigationDir, appLog, syslog);
    const auto timelineResult = investigation.projectTimeline();
    ASSERT_TRUE(timelineResult.hasValue());
    ASSERT_EQ(2U, timelineResult->events.size());

    CorrelationSuggestionQuery query;
    const auto result = CorrelationEngine::computeSuggestions(investigation.manifest().id, timelineResult->events, {},
                                                            {}, query);
    ASSERT_EQ(1, result.total);
    ASSERT_EQ(1U, result.suggestions.size());
    EXPECT_EQ(CorrelationKey::RequestId, result.suggestions[0].matchedKey);
    EXPECT_EQ("abc-123", result.suggestions[0].matchedValue);
    EXPECT_EQ("exact_key_match", result.suggestions[0].ruleId);
    EXPECT_FALSE(result.suggestions[0].summary.empty());
    EXPECT_TRUE(result.suggestions[0].sourceArtifactName == "app.log"
                || result.suggestions[0].sourceArtifactName == "syslog");
    EXPECT_TRUE(result.suggestions[0].targetArtifactName == "app.log"
                || result.suggestions[0].targetArtifactName == "syslog");
    EXPECT_NE(result.suggestions[0].sourceArtifactName, result.suggestions[0].targetArtifactName);

    removeDirectoryTree(investigationDir);
}

TEST(CorrelationEngineTest, SameArtifactPairProducesNoSuggestion)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_corr_same"));
    const Path appLog(logscope::gtest::uniqueTestPath("_app_same.log"));

    writeFile(appLog,
              "2026-08-06T10:00:00 ERROR request_id=abc-123 first\n"
              "2026-08-06T10:00:01 ERROR request_id=abc-123 second\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "same-artifact";

    const auto createResult = Investigation::create(investigationDir, createRequest);
    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest appRequest;
    appRequest.type = "log";
    appRequest.name = "app.log";
    appRequest.sourceFile = appLog;
    appRequest.source = ArtifactSource{"upload", "app.log"};
    ASSERT_TRUE(investigation.addArtifact(appRequest));

    const auto timelineResult = investigation.projectTimeline();
    ASSERT_TRUE(timelineResult.hasValue());
    ASSERT_GE(timelineResult->events.size(), 2U);

    CorrelationSuggestionQuery query;
    const auto result = CorrelationEngine::computeSuggestions(investigation.manifest().id, timelineResult->events, {},
                                                            {}, query);
    EXPECT_EQ(0, result.total);

    removeDirectoryTree(investigationDir);
}

TEST(CorrelationEngineTest, ProximityFilterExcludesDistantMatches)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_corr_prox"));
    const Path appLog(logscope::gtest::uniqueTestPath("_app_prox.log"));
    const Path syslog(logscope::gtest::uniqueTestPath("_sys_prox.log"));

    writeFile(appLog, "2026-08-06T10:00:00 ERROR request_id=abc-123 near\n");
    writeFile(syslog, "2026-08-06T10:01:00 WARNING request_id=abc-123 far\n");

    Investigation investigation = makeInvestigationWithLogs(investigationDir, appLog, syslog);
    const auto timelineResult = investigation.projectTimeline();
    ASSERT_TRUE(timelineResult.hasValue());

    CorrelationSuggestionQuery query;
    const auto result = CorrelationEngine::computeSuggestions(investigation.manifest().id, timelineResult->events, {},
                                                            {}, query);
    EXPECT_EQ(0, result.total);

    removeDirectoryTree(investigationDir);
}

TEST(CorrelationEngineTest, MissingTimestampStillMatchesOnKey)
{
    TimelineEvent first;
    first.id = "evt-a";
    first.artifactId = "art-a";
    first.timestamp = "";
    first.message = "request_id=abc-123";
    first.source.artifactName = "app.log";

    TimelineEvent second;
    second.id = "evt-b";
    second.artifactId = "art-b";
    second.timestamp = "2026-08-06T10:00:01";
    second.message = "request_id=abc-123";
    second.source.artifactName = "syslog";

    CorrelationSuggestionQuery query;
    const auto result =
        CorrelationEngine::computeSuggestions("inv", {first, second}, {}, {}, query);
    ASSERT_EQ(1, result.total);
    EXPECT_FALSE(result.suggestions[0].timeDeltaMs.has_value());
}

TEST(InvestigationCorrelationSuggestionTest, AcceptCreatesRelatedEvidenceLink)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_corr_accept"));
    const Path appLog(logscope::gtest::uniqueTestPath("_app_accept.log"));
    const Path syslog(logscope::gtest::uniqueTestPath("_sys_accept.log"));

    writeFile(appLog, "2026-08-06T10:00:00 ERROR request_id=abc-123 app error\n");
    writeFile(syslog, "2026-08-06T10:00:01 WARNING request_id=abc-123 syslog warn\n");

    Investigation investigation = makeInvestigationWithLogs(investigationDir, appLog, syslog);
    const auto suggestionsResult = investigation.listCorrelationSuggestions({});
    ASSERT_TRUE(suggestionsResult.hasValue());
    ASSERT_EQ(1, suggestionsResult->total);

    const std::string suggestionId = suggestionsResult->suggestions[0].id;
    const auto acceptResult = investigation.acceptCorrelationSuggestion(suggestionId, std::nullopt, std::nullopt);
    ASSERT_TRUE(acceptResult.hasValue());
    EXPECT_EQ(EvidenceLinkType::Related, acceptResult->type);
    EXPECT_EQ(1U, investigation.manifest().evidenceLinks.size());

    const auto afterAccept = investigation.listCorrelationSuggestions({});
    ASSERT_TRUE(afterAccept.hasValue());
    EXPECT_EQ(0, afterAccept->total);

    removeDirectoryTree(investigationDir);
}

TEST(InvestigationCorrelationSuggestionTest, ExistingLinkSuppressesPairSuggestions)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_corr_dedup"));
    const Path appLog(logscope::gtest::uniqueTestPath("_app_dedup.log"));
    const Path syslog(logscope::gtest::uniqueTestPath("_sys_dedup.log"));

    writeFile(appLog, "2026-08-06T10:00:00 ERROR request_id=abc-123 trace_id=trace-1\n");
    writeFile(syslog, "2026-08-06T10:00:01 WARNING request_id=abc-123 trace_id=trace-1\n");

    Investigation investigation = makeInvestigationWithLogs(investigationDir, appLog, syslog);
    const auto timelineResult = investigation.projectTimeline();
    ASSERT_TRUE(timelineResult.hasValue());
    ASSERT_GE(timelineResult->events.size(), 2U);

    EvidenceLinkCreateRequest linkRequest;
    linkRequest.type = EvidenceLinkType::Related;
    linkRequest.source.kind = "timeline_event";
    linkRequest.source.eventId = timelineResult->events[0].id;
    linkRequest.target.kind = "timeline_event";
    linkRequest.target.eventId = timelineResult->events[1].id;
    ASSERT_TRUE(investigation.addEvidenceLink(linkRequest));

    const auto suggestionsResult = investigation.listCorrelationSuggestions({});
    ASSERT_TRUE(suggestionsResult.hasValue());
    EXPECT_EQ(0, suggestionsResult->total);

    removeDirectoryTree(investigationDir);
}

TEST(InvestigationCorrelationSuggestionTest, DismissedSuggestionIdsAreFiltered)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_corr_dismiss"));
    const Path appLog(logscope::gtest::uniqueTestPath("_app_dismiss.log"));
    const Path syslog(logscope::gtest::uniqueTestPath("_sys_dismiss.log"));

    writeFile(appLog, "2026-08-06T10:00:00 ERROR request_id=abc-123 app error\n");
    writeFile(syslog, "2026-08-06T10:00:01 WARNING request_id=abc-123 syslog warn\n");

    Investigation investigation = makeInvestigationWithLogs(investigationDir, appLog, syslog);
    const auto suggestionsResult = investigation.listCorrelationSuggestions({});
    ASSERT_TRUE(suggestionsResult.hasValue());
    ASSERT_EQ(1, suggestionsResult->total);

    std::unordered_set<std::string> dismissed;
    dismissed.insert(suggestionsResult->suggestions[0].id);

    const auto filtered = investigation.listCorrelationSuggestions({}, dismissed);
    ASSERT_TRUE(filtered.hasValue());
    EXPECT_EQ(0, filtered->total);

    removeDirectoryTree(investigationDir);
}
