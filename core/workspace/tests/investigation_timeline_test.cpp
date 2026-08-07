/**
 * @file investigation_timeline_test.cpp
 * @brief Unit tests for investigation timeline projection (Story 3).
 */

#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "artifact_projector.hpp"
#include "gtest_temp_path.hpp"
#include "workspace.hpp"

using scope::foundation::Path;
using scope::workspace::ArtifactIngestRequest;
using scope::workspace::ArtifactSource;
using scope::workspace::Investigation;
using scope::workspace::InvestigationCreateRequest;
using scope::workspace::TimelineProjectionOptions;
using scope::workspace::TimelineSortOrder;

namespace
{

void writeFile(const Path& path, const std::string& contents)
{
    std::ofstream stream(path.string(), std::ios::binary | std::ios::trunc);
    stream << contents;
}

} // namespace

TEST(InvestigationTimelineTest, ProjectsChronologicalLogEventsAcrossArtifacts)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_timeline"));
    const Path appLog(logscope::gtest::uniqueTestPath("_app.log"));
    const Path syslog(logscope::gtest::uniqueTestPath("_sys.log"));

    writeFile(appLog, "2026-08-01T10:00:00 first app event\n2026-08-01T10:00:05 second app event\n");
    writeFile(syslog, "2026-08-01T10:00:02 syslog event\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "timeline-merge";

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
    ASSERT_EQ(3U, timelineResult->events.size());
    EXPECT_EQ("log.line", timelineResult->events[0].eventType);
    EXPECT_EQ("first app event", timelineResult->events[0].message);
    EXPECT_EQ("syslog", timelineResult->events[1].source.artifactName);
    EXPECT_EQ("second app event", timelineResult->events[2].message);
    EXPECT_EQ(1U, timelineResult->events[0].source.lineNumber);
}

TEST(InvestigationTimelineTest, StableTimelineEventIdIsDeterministic)
{
    const std::string first =
        scope::workspace::makeTimelineEventId("inv", "artifact", 0U, "2026-08-01T10:00:00Z", "log.line");
    const std::string second =
        scope::workspace::makeTimelineEventId("inv", "artifact", 0U, "2026-08-01T10:00:00Z", "log.line");

    EXPECT_EQ(first, second);
    EXPECT_FALSE(first.empty());
}

TEST(InvestigationTimelineTest, CoreArtifactProducesAttachedMarker)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_timeline_core"));
    const Path coreFile(logscope::gtest::uniqueTestPath("_dump.core"));

    writeFile(coreFile, std::string(16U, '\0'));

    InvestigationCreateRequest createRequest;
    createRequest.name = "core-marker";

    const auto createResult = Investigation::create(investigationDir, createRequest);

    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest coreRequest;
    coreRequest.type = "core";
    coreRequest.name = "dump.core";
    coreRequest.sourceFile = coreFile;
    coreRequest.source = ArtifactSource{"upload", "dump.core"};

    const auto artifactResult = investigation.addArtifact(coreRequest);

    ASSERT_TRUE(artifactResult.hasValue());

    const auto timelineResult = investigation.projectTimeline();

    ASSERT_TRUE(timelineResult.hasValue());
    ASSERT_EQ(1U, timelineResult->events.size());
    EXPECT_EQ("artifact.attached", timelineResult->events[0].eventType);
    EXPECT_EQ("core", timelineResult->events[0].source.artifactType);
    EXPECT_NE(std::string::npos, timelineResult->events[0].message.find("Core dump attached"));
}

TEST(InvestigationTimelineTest, PaginationTruncatesTimeline)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_timeline_page"));
    const Path appLog(logscope::gtest::uniqueTestPath("_page.log"));

    std::ostringstream logBody;

    for (int index = 0; index < 5; ++index)
    {
        logBody << "2026-08-01T10:00:0" << index << " event " << index << '\n';
    }

    writeFile(appLog, logBody.str());

    InvestigationCreateRequest createRequest;
    createRequest.name = "timeline-page";

    const auto createResult = Investigation::create(investigationDir, createRequest);

    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest appRequest;
    appRequest.type = "log";
    appRequest.name = "app.log";
    appRequest.sourceFile = appLog;
    appRequest.source = ArtifactSource{"upload", "app.log"};

    ASSERT_TRUE(investigation.addArtifact(appRequest));

    TimelineProjectionOptions options;
    options.limit = 2U;

    const auto timelineResult = investigation.projectTimeline(options);

    ASSERT_TRUE(timelineResult.hasValue());
    EXPECT_EQ(2U, timelineResult->events.size());
    EXPECT_TRUE(timelineResult->truncated);
    EXPECT_EQ(5U, timelineResult->totalMatched);
}

TEST(InvestigationTimelineTest, DescendingOrderReversesChronology)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_timeline_desc"));
    const Path appLog(logscope::gtest::uniqueTestPath("_desc.log"));

    writeFile(appLog, "2026-08-01T10:00:00 first\n2026-08-01T10:00:05 second\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "timeline-desc";

    const auto createResult = Investigation::create(investigationDir, createRequest);

    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest appRequest;
    appRequest.type = "log";
    appRequest.name = "app.log";
    appRequest.sourceFile = appLog;
    appRequest.source = ArtifactSource{"upload", "app.log"};

    ASSERT_TRUE(investigation.addArtifact(appRequest));

    TimelineProjectionOptions options;
    options.order = TimelineSortOrder::Descending;

    const auto timelineResult = investigation.projectTimeline(options);

    ASSERT_TRUE(timelineResult.hasValue());
    ASSERT_EQ(2U, timelineResult->events.size());
    EXPECT_EQ("second", timelineResult->events[0].message);
    EXPECT_EQ("first", timelineResult->events[1].message);
}

TEST(InvestigationTimelineTest, PreservesSubSecondPrecisionAndMessageBoundaries)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_timeline_subsecond"));
    const Path appLog(logscope::gtest::uniqueTestPath("_subsecond.log"));

    writeFile(appLog,
              "2026-07-28T09:15:01.101Z FE01 INFO ManagerA started\n"
              "2026-07-28T09:15:01.842Z FE01 INFO ManagerB finished\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "timeline-subsecond";

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
    ASSERT_EQ(2U, timelineResult->events.size());
    EXPECT_EQ("2026-07-28T09:15:01.101Z", timelineResult->events[0].timestamp);
    EXPECT_EQ("2026-07-28T09:15:01.842Z", timelineResult->events[1].timestamp);
    EXPECT_EQ("FE01 INFO ManagerA started", timelineResult->events[0].message);
    EXPECT_EQ("FE01 INFO ManagerB finished", timelineResult->events[1].message);
    EXPECT_NE(timelineResult->events[0].timestamp, timelineResult->events[1].timestamp);
    EXPECT_FALSE(timelineResult->events[0].message.empty());
    EXPECT_NE('.', timelineResult->events[0].message.front());
}
