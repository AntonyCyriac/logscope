/**
 * @file investigation_evidence_link_test.cpp
 * @brief Unit tests for evidence links (Story 5 / v2.7.0).
 */

#include <fstream>
#include <filesystem>

#include <gtest/gtest.h>

#include "gtest_temp_path.hpp"
#include "investigation_manifest_io.hpp"
#include "workspace.hpp"

using scope::foundation::ErrorCode;
using scope::foundation::Path;
using scope::workspace::ArtifactIngestRequest;
using scope::workspace::ArtifactSource;
using scope::workspace::EvidenceLinkCreateRequest;
using scope::workspace::EvidenceLinkStatus;
using scope::workspace::EvidenceLinkType;
using scope::workspace::Investigation;
using scope::workspace::InvestigationCreateRequest;
using scope::workspace::loadManifest;
using scope::workspace::saveManifest;

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

} // namespace

TEST(InvestigationEvidenceLinkTest, ManifestV1RoundTripWithoutLinksStaysV1)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_manifest_v1"));
    const Path logSource(logscope::gtest::uniqueTestPath("_app.log"));

    writeFile(logSource, "2026-08-06T10:00:00 app event\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "v1-manifest";

    const auto createResult = Investigation::create(investigationDir, createRequest);
    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);
    EXPECT_EQ(1, investigation.manifest().schemaVersion);
    EXPECT_TRUE(investigation.manifest().evidenceLinks.empty());

    ArtifactIngestRequest logRequest;
    logRequest.type = "log";
    logRequest.name = "app.log";
    logRequest.sourceFile = logSource;
    logRequest.source = ArtifactSource{"upload", "app.log"};
    ASSERT_TRUE(investigation.addArtifact(logRequest));
    ASSERT_TRUE(investigation.persist());

    const auto reloadResult = loadManifest(investigationDir);
    ASSERT_TRUE(reloadResult.hasValue());
    EXPECT_EQ(1, reloadResult->schemaVersion);
    EXPECT_TRUE(reloadResult->evidenceLinks.empty());

    removeDirectoryTree(investigationDir);
}

TEST(InvestigationEvidenceLinkTest, AddLinkBumpsManifestToV2AndRoundTrips)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_manifest_v2"));
    const Path appLog(logscope::gtest::uniqueTestPath("_app.log"));
    const Path syslog(logscope::gtest::uniqueTestPath("_sys.log"));

    writeFile(appLog, "2026-08-06T10:00:00 first app\n");
    writeFile(syslog, "2026-08-06T10:00:01 syslog line\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "v2-links";

    const auto createResult = Investigation::create(investigationDir, createRequest);
    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest appRequest;
    appRequest.type = "log";
    appRequest.name = "app.log";
    appRequest.sourceFile = appLog;
    appRequest.source = ArtifactSource{"upload", "app.log"};
    ASSERT_TRUE(investigation.addArtifact(appRequest));

    ArtifactIngestRequest syslogRequest;
    syslogRequest.type = "log";
    syslogRequest.name = "syslog";
    syslogRequest.sourceFile = syslog;
    syslogRequest.source = ArtifactSource{"upload", "syslog"};
    ASSERT_TRUE(investigation.addArtifact(syslogRequest));

    const auto timelineResult = investigation.projectTimeline();
    ASSERT_TRUE(timelineResult.hasValue());
    ASSERT_GE(timelineResult->events.size(), 2U);

    EvidenceLinkCreateRequest linkRequest;
    linkRequest.type = EvidenceLinkType::Related;
    linkRequest.source.kind = "timeline_event";
    linkRequest.source.eventId = timelineResult->events[0].id;
    linkRequest.target.kind = "timeline_event";
    linkRequest.target.eventId = timelineResult->events[1].id;
    linkRequest.note = "same incident window";

    const auto addResult = investigation.addEvidenceLink(linkRequest);
    ASSERT_TRUE(addResult.hasValue());
    EXPECT_EQ(EvidenceLinkStatus::Active, addResult->status);
    EXPECT_EQ(2, investigation.manifest().schemaVersion);

    const auto reloadResult = loadManifest(investigationDir);
    ASSERT_TRUE(reloadResult.hasValue());
    EXPECT_EQ(2, reloadResult->schemaVersion);
    ASSERT_EQ(1U, reloadResult->evidenceLinks.size());
    EXPECT_EQ("RELATED", scope::workspace::evidenceLinkTypeToString(reloadResult->evidenceLinks[0].type));
    EXPECT_EQ("same incident window", *reloadResult->evidenceLinks[0].note);

    removeDirectoryTree(investigationDir);
}

TEST(InvestigationEvidenceLinkTest, RejectsDuplicateLinkWith409Semantics)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_dup_link"));
    const Path appLog(logscope::gtest::uniqueTestPath("_app.log"));
    const Path syslog(logscope::gtest::uniqueTestPath("_sys.log"));

    writeFile(appLog, "2026-08-06T10:00:00 first app\n");
    writeFile(syslog, "2026-08-06T10:00:01 syslog line\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "dup-links";

    const auto createResult = Investigation::create(investigationDir, createRequest);
    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest appRequest;
    appRequest.type = "log";
    appRequest.name = "app.log";
    appRequest.sourceFile = appLog;
    appRequest.source = ArtifactSource{"upload", "app.log"};
    ASSERT_TRUE(investigation.addArtifact(appRequest));

    ArtifactIngestRequest syslogRequest;
    syslogRequest.type = "log";
    syslogRequest.name = "syslog";
    syslogRequest.sourceFile = syslog;
    syslogRequest.source = ArtifactSource{"upload", "syslog"};
    ASSERT_TRUE(investigation.addArtifact(syslogRequest));

    const auto timelineResult = investigation.projectTimeline();
    ASSERT_TRUE(timelineResult.hasValue());

    EvidenceLinkCreateRequest linkRequest;
    linkRequest.type = EvidenceLinkType::Related;
    linkRequest.source.kind = "timeline_event";
    linkRequest.source.eventId = timelineResult->events[0].id;
    linkRequest.target.kind = "timeline_event";
    linkRequest.target.eventId = timelineResult->events[1].id;

    ASSERT_TRUE(investigation.addEvidenceLink(linkRequest));

    const auto duplicateResult = investigation.addEvidenceLink(linkRequest);
    ASSERT_FALSE(duplicateResult.hasValue());
    EXPECT_EQ(ErrorCode::DuplicateEvidenceLink, duplicateResult.error().code());

    removeDirectoryTree(investigationDir);
}

TEST(InvestigationEvidenceLinkTest, ListsStaleLinkWhenEndpointMissingFromProjection)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_stale_link"));
    const Path appLog(logscope::gtest::uniqueTestPath("_app.log"));

    writeFile(appLog, "2026-08-06T10:00:00 only event\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "stale-links";

    const auto createResult = Investigation::create(investigationDir, createRequest);
    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest appRequest;
    appRequest.type = "log";
    appRequest.name = "app.log";
    appRequest.sourceFile = appLog;
    appRequest.source = ArtifactSource{"upload", "app.log"};
    const auto appArtifact = investigation.addArtifact(appRequest);
    ASSERT_TRUE(appArtifact.hasValue());

    const auto timelineResult = investigation.projectTimeline();
    ASSERT_TRUE(timelineResult.hasValue());
    ASSERT_FALSE(timelineResult->events.empty());

    EvidenceLinkCreateRequest linkRequest;
    linkRequest.type = EvidenceLinkType::Supports;
    linkRequest.source.kind = "timeline_event";
    linkRequest.source.eventId = timelineResult->events[0].id;
    linkRequest.target.kind = "timeline_event";
    linkRequest.target.eventId = "missing-event-id";
    linkRequest.note = "orphan target";

    const auto invalidResult = investigation.addEvidenceLink(linkRequest);
    ASSERT_FALSE(invalidResult.hasValue());
    EXPECT_EQ(ErrorCode::InvalidLinkTarget, invalidResult.error().code());

    linkRequest.target.eventId = timelineResult->events[0].id;
    linkRequest.source.eventId = "missing-event-id";
    const auto invalidSource = investigation.addEvidenceLink(linkRequest);
    ASSERT_FALSE(invalidSource.hasValue());
    EXPECT_EQ(ErrorCode::InvalidLinkTarget, invalidSource.error().code());

    scope::workspace::InvestigationManifest manifest = investigation.manifest();
    scope::workspace::EvidenceLink staleLink;
    staleLink.id = "stale-link-id";
    staleLink.type = EvidenceLinkType::Related;
    staleLink.source.kind = "timeline_event";
    staleLink.source.eventId = timelineResult->events[0].id;
    staleLink.target.kind = "timeline_event";
    staleLink.target.eventId = "gone-event-id";
    staleLink.createdAt = "2026-08-06T12:00:00Z";
    manifest.evidenceLinks.push_back(staleLink);
    manifest.schemaVersion = 2;
    ASSERT_TRUE(saveManifest(investigationDir, manifest));

    const auto reopenResult = Investigation::open(investigationDir);
    ASSERT_TRUE(reopenResult.hasValue());

    const auto linksResult = reopenResult->listEvidenceLinks();
    ASSERT_TRUE(linksResult.hasValue());
    ASSERT_EQ(1U, linksResult->size());
    EXPECT_EQ(EvidenceLinkStatus::Stale, linksResult->front().status);

    removeDirectoryTree(investigationDir);
}
