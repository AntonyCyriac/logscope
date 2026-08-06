/**
 * @file pstack_crash_analyzer_test.cpp
 * @brief Unit tests for pstack crash analysis (Story 4).
 */

#include <fstream>

#include <gtest/gtest.h>

#include "crash_analyzer.hpp"
#include "gtest_temp_path.hpp"
#include "workspace.hpp"

using scope::foundation::Path;
using scope::workspace::ArtifactIngestRequest;
using scope::workspace::ArtifactSource;
using scope::workspace::CrashAnalysisStatus;
using scope::workspace::Investigation;
using scope::workspace::InvestigationCreateRequest;

namespace
{

void writeFile(const Path& path, const std::string& contents)
{
    std::ofstream stream(path.string(), std::ios::binary | std::ios::trunc);
    stream << contents;
}

} // namespace

TEST(PstackCrashAnalyzerTest, ParsesSyntheticFixtureThreadsAndSignal)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_crash"));
    const Path pstackFile(logscope::gtest::uniqueTestPath("_pstack.txt"));

    writeFile(pstackFile, R"(Thread 1 (LWP 12345):
#0  0x00005555555a2b10 in SessionManager::create (this=0x0) at session_manager.cpp:42
#1  0x00005555555a1c01 in main (argc=1, argv=0x7fffffffe5a8) at app.cpp:18

Program received signal SIGSEGV, Segmentation fault.
[Switching to thread 1 (LWP 12345)]
)");

    InvestigationCreateRequest createRequest;
    createRequest.name = "crash-test";

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

    const auto crashResult = investigation.analyzeCrash(artifactResult->id);

    ASSERT_TRUE(crashResult.hasValue());
    EXPECT_EQ(CrashAnalysisStatus::Complete, crashResult->status);
    EXPECT_EQ("pstack", crashResult->artifactType);
    EXPECT_EQ(artifactResult->id, crashResult->artifactId);
    EXPECT_TRUE(crashResult->signal.has_value());
    EXPECT_EQ("SIGSEGV", *crashResult->signal);
    EXPECT_TRUE(crashResult->faultThreadId.has_value());
    EXPECT_EQ("1", *crashResult->faultThreadId);
    ASSERT_EQ(1U, crashResult->threads.size());
    EXPECT_TRUE(crashResult->threads.front().isFaultThread);
    ASSERT_GE(crashResult->threads.front().frames.size(), 1U);
    EXPECT_EQ("SessionManager::create", crashResult->threads.front().frames.front().symbol);
    EXPECT_NE(std::string::npos, crashResult->summary.find("SessionManager::create"));
    EXPECT_FALSE(crashResult->observations.empty());
}

TEST(PstackCrashAnalyzerTest, StableCrashReportIdIsDeterministic)
{
    const std::string first =
        scope::workspace::makeCrashReportId("inv", "artifact", "pstack-v1");
    const std::string second =
        scope::workspace::makeCrashReportId("inv", "artifact", "pstack-v1");

    EXPECT_EQ(first, second);
    EXPECT_FALSE(first.empty());
}

TEST(PstackCrashAnalyzerTest, LogArtifactIsNotAnalyzable)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_crash_log"));
    const Path logFile(logscope::gtest::uniqueTestPath("_app.log"));

    writeFile(logFile, "2026-08-01T10:00:00 app started\n");

    InvestigationCreateRequest createRequest;
    createRequest.name = "crash-log";

    const auto createResult = Investigation::create(investigationDir, createRequest);

    ASSERT_TRUE(createResult.hasValue());

    Investigation investigation = std::move(*createResult);

    ArtifactIngestRequest logRequest;
    logRequest.type = "log";
    logRequest.name = "app.log";
    logRequest.sourceFile = logFile;
    logRequest.source = ArtifactSource{"upload", "app.log"};

    const auto artifactResult = investigation.addArtifact(logRequest);

    ASSERT_TRUE(artifactResult.hasValue());

    const auto crashResult = investigation.analyzeCrash(artifactResult->id);

    ASSERT_FALSE(crashResult.hasValue());
    EXPECT_EQ("ARTIFACT_NOT_ANALYZABLE", crashResult.error().message());
}
