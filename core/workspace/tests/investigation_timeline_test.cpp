/**
 * @file investigation_timeline_test.cpp
 * @brief Unit tests for investigation timeline projection (Story 3).
 */

#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "artifact_projector.hpp"
#include "crash_report.hpp"
#include "crash_summary_timeline.hpp"
#include "gtest_temp_path.hpp"
#include "workspace.hpp"

using scope::foundation::Path;
using scope::workspace::ArtifactIngestRequest;
using scope::workspace::ArtifactRecord;
using scope::workspace::ArtifactSource;
using scope::workspace::CrashAnalysisStatus;
using scope::workspace::CrashReport;
using scope::workspace::Investigation;
using scope::workspace::InvestigationCreateRequest;
using scope::workspace::TimelineEvent;
using scope::workspace::TimelineProjectionOptions;
using scope::workspace::TimelineSortOrder;
using scope::workspace::makeCrashSummaryTimelineEvent;

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

TEST(InvestigationTimelineTest, CoreArtifactProducesCrashSummaryOnTimeline)
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
    EXPECT_EQ("crash.summary", timelineResult->events[0].eventType);
    EXPECT_EQ("core", timelineResult->events[0].source.artifactType);
    EXPECT_NE(std::string::npos, timelineResult->events[0].message.find("dump.core"));
    EXPECT_EQ("failed", timelineResult->events[0].metadata.at("status"));
}

TEST(InvestigationTimelineTest, PstackArtifactProducesCrashSummaryOnTimeline)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_timeline_pstack"));
    const Path pstackFile(logscope::gtest::uniqueTestPath("_pstack.txt"));

    writeFile(pstackFile, R"(Thread 1 (LWP 12345):
#0  0x00005555555a2b10 in SessionManager::create (this=0x0) at session_manager.cpp:42

Program received signal SIGSEGV, Segmentation fault.
[Switching to thread 1 (LWP 12345)]
)");

    InvestigationCreateRequest createRequest;
    createRequest.name = "pstack-timeline";

    const auto createResult = Investigation::create(investigationDir, createRequest);

    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest pstackRequest;
    pstackRequest.type = "pstack";
    pstackRequest.name = "pstack.txt";
    pstackRequest.sourceFile = pstackFile;
    pstackRequest.source = ArtifactSource{"upload", "pstack.txt"};

    const auto artifactResult = investigation.addArtifact(pstackRequest);

    ASSERT_TRUE(artifactResult.hasValue());

    const auto timelineResult = investigation.projectTimeline();

    ASSERT_TRUE(timelineResult.hasValue());
    ASSERT_EQ(1U, timelineResult->events.size());
    EXPECT_EQ("crash.summary", timelineResult->events[0].eventType);
    EXPECT_EQ("ready", timelineResult->events[0].metadata.at("status"));
    EXPECT_EQ("SIGSEGV", timelineResult->events[0].metadata.at("signal"));
    EXPECT_NE(std::string::npos, timelineResult->events[0].message.find("SessionManager::create"));
    EXPECT_TRUE(timelineResult->events[0].severity.has_value());
}

TEST(InvestigationTimelineTest, MultiplePstackArtifactsProduceMultipleCrashSummaries)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_timeline_multi_pstack"));
    const Path firstPstack(logscope::gtest::uniqueTestPath("_pstack_a.txt"));
    const Path secondPstack(logscope::gtest::uniqueTestPath("_pstack_b.txt"));

    writeFile(firstPstack, R"(Program received signal SIGSEGV, Segmentation fault.
Thread 1 (LWP 1):
#0  0x1 in first () at first.cpp:1
)");
    writeFile(secondPstack, R"(Program received signal SIGABRT, Aborted.
Thread 1 (LWP 1):
#0  0x2 in second () at second.cpp:2
)");

    InvestigationCreateRequest createRequest;
    createRequest.name = "multi-pstack";

    const auto createResult = Investigation::create(investigationDir, createRequest);

    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest firstRequest;
    firstRequest.type = "pstack";
    firstRequest.name = "a.txt";
    firstRequest.sourceFile = firstPstack;
    firstRequest.source = ArtifactSource{"upload", "a.txt"};
    ASSERT_TRUE(investigation.addArtifact(firstRequest));

    ArtifactIngestRequest secondRequest;
    secondRequest.type = "pstack";
    secondRequest.name = "b.txt";
    secondRequest.sourceFile = secondPstack;
    secondRequest.source = ArtifactSource{"upload", "b.txt"};
    ASSERT_TRUE(investigation.addArtifact(secondRequest));

    const auto timelineResult = investigation.projectTimeline();

    ASSERT_TRUE(timelineResult.hasValue());
    ASSERT_EQ(2U, timelineResult->events.size());
    EXPECT_EQ("crash.summary", timelineResult->events[0].eventType);
    EXPECT_EQ("crash.summary", timelineResult->events[1].eventType);
    EXPECT_NE(timelineResult->events[0].id, timelineResult->events[1].id);
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

TEST(InvestigationTimelineTest, IngestCapturesSourceFileModifiedAt)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_timeline_mtime"));
    const Path appLog(logscope::gtest::uniqueTestPath("_mtime.log"));

    writeFile(appLog, "2026-08-01T10:00:00 event\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "mtime-capture";

    const auto createResult = Investigation::create(investigationDir, createRequest);

    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest appRequest;
    appRequest.type = "log";
    appRequest.name = "app.log";
    appRequest.sourceFile = appLog;
    appRequest.source = ArtifactSource{"upload", "app.log"};

    const auto artifactResult = investigation.addArtifact(appRequest);

    ASSERT_TRUE(artifactResult.hasValue());
    EXPECT_FALSE(artifactResult->sourceModifiedAt.empty());
}

TEST(InvestigationTimelineTest, CrashSummaryPrefersSourceModifiedAtOverImportedAt)
{
    ArtifactRecord artifact;
    artifact.id = "artifact-1";
    artifact.type = "pstack";
    artifact.name = "app.pstack";
    artifact.importedAt = "2026-08-13T03:23:28.568757549Z";
    artifact.sourceModifiedAt = "2026-05-06T10:32:11Z";

    CrashReport report;
    report.id = "crash-1";
    report.artifactId = artifact.id;
    report.artifactType = "pstack";
    report.status = CrashAnalysisStatus::Ready;
    report.signal = "SIGSEGV";
    report.summary = "SIGSEGV — thread 18";

    const std::optional<TimelineEvent> event = makeCrashSummaryTimelineEvent("inv-1", artifact, report);

    ASSERT_TRUE(event.has_value());
    EXPECT_EQ("2026-05-06T10:32:11Z", event->timestamp);
    EXPECT_EQ("source_mtime", event->metadata.at("timestampSource"));
    EXPECT_EQ("true", event->metadata.at("timestampApproximate"));
}

TEST(InvestigationTimelineTest, UnrecognisedLogTimestampsEmitSkipWarning)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_timeline_skip"));
    const Path appLog(logscope::gtest::uniqueTestPath("_skip.log"));

    writeFile(appLog, "05-05-2026 12:57:48  100-10   clear   \"registered\"\n"
                       "05-05-2026 12:58:28  107-46   major   \"pid died\"\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "timeline-skip";

    const auto createResult = Investigation::create(investigationDir, createRequest);

    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest appRequest;
    appRequest.type = "log";
    appRequest.name = "platform.log";
    appRequest.sourceFile = appLog;
    appRequest.source = ArtifactSource{"upload", "platform.log"};

    ASSERT_TRUE(investigation.addArtifact(appRequest));

    const auto timelineResult = investigation.projectTimeline();

    ASSERT_TRUE(timelineResult.hasValue());
    EXPECT_TRUE(timelineResult->events.empty());
    ASSERT_EQ(1U, timelineResult->artifactStats.size());
    EXPECT_EQ(2U, timelineResult->artifactStats[0].linesRead);
    EXPECT_EQ(2U, timelineResult->artifactStats[0].linesSkippedNoTimestamp);
    ASSERT_FALSE(timelineResult->warnings.empty());
    EXPECT_NE(std::string::npos, timelineResult->warnings[0].find("skipped"));
}

TEST(InvestigationTimelineTest, PstackIngestUsesSourceModifiedAtForCrashSummary)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_timeline_pstack_mtime"));
    const Path pstackFile(logscope::gtest::uniqueTestPath("_pstack_mtime.txt"));

    writeFile(pstackFile, R"(Program received signal SIGSEGV, Segmentation fault.
Thread 1 (LWP 1):
#0  0x1 in fault () at fault.cpp:1
)");

    InvestigationCreateRequest createRequest;
    createRequest.name = "pstack-mtime";

    const auto createResult = Investigation::create(investigationDir, createRequest);

    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest pstackRequest;
    pstackRequest.type = "pstack";
    pstackRequest.name = "app.pstack";
    pstackRequest.sourceFile = pstackFile;
    pstackRequest.source = ArtifactSource{"upload", "app.pstack"};

    const auto artifactResult = investigation.addArtifact(pstackRequest);

    ASSERT_TRUE(artifactResult.hasValue());
    EXPECT_FALSE(artifactResult->sourceModifiedAt.empty());

    const auto timelineResult = investigation.projectTimeline();

    ASSERT_TRUE(timelineResult.hasValue());
    ASSERT_EQ(1U, timelineResult->events.size());
    EXPECT_EQ("crash.summary", timelineResult->events[0].eventType);
    EXPECT_EQ(artifactResult->sourceModifiedAt, timelineResult->events[0].timestamp);
    EXPECT_EQ("source_mtime", timelineResult->events[0].metadata.at("timestampSource"));
}
