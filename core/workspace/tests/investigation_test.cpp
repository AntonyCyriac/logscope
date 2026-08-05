/**
 * @file investigation_test.cpp
 * @brief Unit tests for Investigation aggregate.
 */

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "gtest_temp_path.hpp"
#include "workspace.hpp"

using scope::foundation::Path;
using scope::workspace::ArtifactIngestRequest;
using scope::workspace::ArtifactSource;
using scope::workspace::Investigation;
using scope::workspace::InvestigationCreateRequest;

namespace
{

void removeDirectoryTree(const Path& directory)
{
    std::error_code errorCode;
    std::filesystem::remove_all(directory.string(), errorCode);
}

} // namespace

TEST(InvestigationTest, RoundTripsCreateAddPersistReopen)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_investigation"));
    const Path logSourceFile(logscope::gtest::uniqueTestPath("_source.log"));

    {
        std::ofstream logStream(logSourceFile.string());
        logStream << "2026-08-04 ERROR service failed\n";
    }

    InvestigationCreateRequest createRequest;
    createRequest.name = "prod-outage";
    createRequest.description = "Story 1 round trip";

    const auto createResult = Investigation::create(investigationDir, createRequest);

    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);
    EXPECT_EQ("prod-outage", investigation.manifest().name);
    EXPECT_EQ("Story 1 round trip", investigation.manifest().description);
    EXPECT_FALSE(investigation.manifest().id.empty());

    ArtifactIngestRequest logRequest;
    logRequest.type = "log";
    logRequest.name = "app.log";
    logRequest.sourceFile = logSourceFile;
    logRequest.source = ArtifactSource{"upload", "app.log"};

    const auto logArtifactResult = investigation.addArtifact(logRequest);

    ASSERT_TRUE(logArtifactResult.hasValue());
    EXPECT_EQ("log", logArtifactResult->type);
    EXPECT_EQ("app.log", logArtifactResult->name);

    ArtifactIngestRequest noteRequest;
    noteRequest.type = "note";
    noteRequest.name = "incident-note";
    noteRequest.noteBody = "Customer reported outage at 14:00.";
    noteRequest.source = ArtifactSource{"inline", "incident-note"};

    const auto noteArtifactResult = investigation.addArtifact(noteRequest);

    ASSERT_TRUE(noteArtifactResult.hasValue());
    EXPECT_EQ("note", noteArtifactResult->type);

    const auto entryArtifactResult = investigation.entryArtifact();

    ASSERT_TRUE(entryArtifactResult.hasValue());
    EXPECT_EQ("log", entryArtifactResult->type);
    EXPECT_EQ(logArtifactResult->id, entryArtifactResult->id);

    const auto entryDataPathResult = investigation.entryArtifactDataPath();

    ASSERT_TRUE(entryDataPathResult.hasValue());
    EXPECT_TRUE(std::filesystem::exists(entryDataPathResult->string()));

    const auto persistResult = investigation.persist();

    ASSERT_TRUE(persistResult.hasValue());
    EXPECT_TRUE(*persistResult);

    const auto reopenResult = Investigation::open(investigationDir);

    ASSERT_TRUE(reopenResult.hasValue());
    EXPECT_EQ(investigation.manifest().id, reopenResult->manifest().id);
    EXPECT_EQ("prod-outage", reopenResult->manifest().name);
    ASSERT_EQ(2U, reopenResult->manifest().artifacts.size());
    EXPECT_EQ(logArtifactResult->id, reopenResult->manifest().primaryArtifactId);

    const auto reopenedEntryResult = reopenResult->entryArtifact();

    ASSERT_TRUE(reopenedEntryResult.hasValue());
    EXPECT_EQ("log", reopenedEntryResult->type);

    std::remove(logSourceFile.string().c_str());
    removeDirectoryTree(investigationDir);
}

TEST(InvestigationTest, AddsPstackAndCoreArtifactsAndResolvesLogById)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_investigation_ms"));
    const Path logSourceFile(logscope::gtest::uniqueTestPath("_app.log"));
    const Path syslogFile(logscope::gtest::uniqueTestPath("_syslog.log"));
    const Path pstackFile(logscope::gtest::uniqueTestPath("_pstack.txt"));
    const Path coreFile(logscope::gtest::uniqueTestPath("_dump.core"));

    {
        std::ofstream logStream(logSourceFile.string());
        logStream << "2026-08-05 ERROR app failed\n";
        std::ofstream syslogStream(syslogFile.string());
        syslogStream << "2026-08-05 ERROR kernel oops\n";
        std::ofstream pstackStream(pstackFile.string());
        pstackStream << "#0 main ()\n";
        std::ofstream coreStream(coreFile.string(), std::ios::binary);
        coreStream << "CORE";
    }

    InvestigationCreateRequest createRequest;
    createRequest.name = "multi-source";

    const auto createResult = Investigation::create(investigationDir, createRequest);
    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest appLogRequest;
    appLogRequest.type = "log";
    appLogRequest.name = "app.log";
    appLogRequest.sourceFile = logSourceFile;
    appLogRequest.source = ArtifactSource{"upload", "app.log"};
    appLogRequest.role = "application";

    const auto appLogResult = investigation.addArtifact(appLogRequest);
    ASSERT_TRUE(appLogResult.hasValue());

    ArtifactIngestRequest syslogRequest;
    syslogRequest.type = "log";
    syslogRequest.name = "syslog";
    syslogRequest.sourceFile = syslogFile;
    syslogRequest.source = ArtifactSource{"upload", "syslog"};
    syslogRequest.role = "system";

    const auto syslogResult = investigation.addArtifact(syslogRequest);
    ASSERT_TRUE(syslogResult.hasValue());
    EXPECT_EQ(appLogResult->id, investigation.manifest().primaryArtifactId);

    ArtifactIngestRequest pstackRequest;
    pstackRequest.type = "pstack";
    pstackRequest.name = "threads.txt";
    pstackRequest.sourceFile = pstackFile;
    pstackRequest.source = ArtifactSource{"upload", "threads.txt"};

    const auto pstackResult = investigation.addArtifact(pstackRequest);
    ASSERT_TRUE(pstackResult.hasValue());
    EXPECT_EQ("pstack", pstackResult->type);

    ArtifactIngestRequest coreRequest;
    coreRequest.type = "core";
    coreRequest.name = "core.dump";
    coreRequest.sourceFile = coreFile;
    coreRequest.source = ArtifactSource{"upload", "core.dump"};

    const auto coreResult = investigation.addArtifact(coreRequest);
    ASSERT_TRUE(coreResult.hasValue());
    EXPECT_EQ("core", coreResult->type);
    EXPECT_FALSE(coreResult->metadata.at("sizeBytes").empty());

    const auto syslogPathResult = investigation.logArtifactDataPath(syslogResult->id);
    ASSERT_TRUE(syslogPathResult.hasValue());
    EXPECT_TRUE(std::filesystem::exists(syslogPathResult->string()));

    const auto rejectEntry = investigation.setEntryArtifact(pstackResult->id);
    EXPECT_FALSE(rejectEntry);

    ASSERT_TRUE(investigation.persist());

    std::remove(logSourceFile.string().c_str());
    std::remove(syslogFile.string().c_str());
    std::remove(pstackFile.string().c_str());
    std::remove(coreFile.string().c_str());
    removeDirectoryTree(investigationDir);
}
