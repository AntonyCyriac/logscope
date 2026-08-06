/**
 * @file pstack_crash_analyzer_test.cpp
 * @brief Unit tests for pstack crash analysis (Story 4).
 */

#include <algorithm>
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
    EXPECT_EQ(CrashAnalysisStatus::Ready, crashResult->status);
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

TEST(PstackCrashAnalyzerTest, SelectsAbortThreadWhenNotListedFirstWithoutSignalLine)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_crash_abort_last"));
    const Path pstackFile(logscope::gtest::uniqueTestPath("_pstack_abort_last.txt"));

    writeFile(pstackFile, R"(Thread 7 (Thread 0x7f2a1c0d1700 (LWP 1849)):
#0  0x00007f2a2b9c1e2b in epoll_wait () from /lib64/libc.so.6
#1  0x00007f2a2c114a70 in evl::EventLoop::run() () from /opt/ims/lib/libims_common.so
#2  0x00007f2a2b9f2ea5 in start_thread () from /lib64/libpthread.so.0

Thread 3 (Thread 0x7f2a1b8c0700 (LWP 1845)):
#0  0x00007f2a2b9c8f4a in pthread_cond_wait () from /lib64/libpthread.so.0
#1  0x00007f2a2c0aa311 in grpc_core::Executor::ThreadMain() () from /opt/ims/lib/libgrpc.so
#2  0x00007f2a2b9f2ea5 in start_thread () from /lib64/libpthread.so.0

Thread 1 (Thread 0x7f2a2d4f8880 (LWP 1842)):
#0  0x00007f2a2b93a37f in raise () from /lib64/libc.so.6
#1  0x00007f2a2b924db5 in abort () from /lib64/libc.so.6
#2  0x00007f2a2bf1c09b in __gnu_cxx::__verbose_terminate_handler () from /lib64/libstdc++.so.6
#3  0x00007f2a2bf2a53c in __cxxabiv1::__terminate () from /lib64/libstdc++.so.6
#4  0x00007f2a2bf2a5a7 in std::terminate () from /lib64/libstdc++.so.6
#5  0x00007f2a2c3311d4 in AppHandler::isAllowed(std::string const&) () from /opt/app/lib/libapp.so
#6  0x0000000000403c21 in main () from /opt/app/bin/app
)");

    InvestigationCreateRequest createRequest;
    createRequest.name = "crash-abort-last";

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
    EXPECT_EQ(CrashAnalysisStatus::Ready, crashResult->status);
    ASSERT_TRUE(crashResult->faultThreadId.has_value());
    EXPECT_EQ("1", *crashResult->faultThreadId);

    const auto faultThread = std::find_if(crashResult->threads.begin(), crashResult->threads.end(),
                                          [](const auto& thread) { return thread.isFaultThread; });

    ASSERT_NE(crashResult->threads.end(), faultThread);
    EXPECT_NE(std::string::npos, crashResult->summary.find("AppHandler::isAllowed"));
    ASSERT_FALSE(crashResult->observations.empty());
    EXPECT_NE(std::string::npos, crashResult->observations.front().find("AppHandler::isAllowed"));
    EXPECT_EQ(std::string::npos, crashResult->observations.front().find("epoll_wait"));
}

TEST(PstackCrashAnalyzerTest, NoFaultThreadWhenAllIdle)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_crash_idle"));
    const Path pstackFile(logscope::gtest::uniqueTestPath("_pstack_idle.txt"));

    writeFile(pstackFile, R"(Thread 4 (Thread 0x7f2a1c0d1700 (LWP 2001)):
#0  0x00007f2a2b9c1e2b in epoll_wait () from /lib64/libc.so.6
#1  0x00007f2a2c114a70 in evl::EventLoop::run() () from /opt/ims/lib/libims_common.so

Thread 2 (Thread 0x7f2a1b8c0700 (LWP 2002)):
#0  0x00007f2a2b9c8f4a in pthread_cond_wait () from /lib64/libpthread.so.0
#1  0x00007f2a2c0aa311 in grpc_core::Executor::ThreadMain() () from /opt/ims/lib/libgrpc.so

Thread 1 (Thread 0x7f2a2d4f8880 (LWP 2000)):
#0  0x00007f2a2b9c1f00 in futex_wait () from /lib64/libc.so.6
#1  0x0000000000403c21 in main () from /opt/app/bin/app
)");

    InvestigationCreateRequest createRequest;
    createRequest.name = "crash-idle";

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
    EXPECT_EQ(CrashAnalysisStatus::Ready, crashResult->status);
    EXPECT_FALSE(crashResult->faultThreadId.has_value());
    ASSERT_FALSE(crashResult->observations.empty());
    EXPECT_NE(std::string::npos,
              crashResult->observations.front().find("No fault thread identified"));

    for (const auto& thread : crashResult->threads)
    {
        EXPECT_FALSE(thread.isFaultThread);
    }
}

TEST(PstackCrashAnalyzerTest, NamesFrameBelowSignalHandlerNotHandler)
{
    const Path investigationDir(logscope::gtest::uniqueTestPath("_crash_sig"));
    const Path pstackFile(logscope::gtest::uniqueTestPath("_pstack_sig.txt"));

    writeFile(pstackFile, R"(Thread 9 (Thread 0x7f2a1c0d1700 (LWP 3101)):
#0  0x00007f2a2b9c1e2b in epoll_wait () from /lib64/libc.so.6
#1  0x00007f2a2c114a70 in evl::EventLoop::run() () from /opt/ims/lib/libims_common.so

Thread 6 (Thread 0x7f2a1b8c0700 (LWP 3105)):
#0  0x00007f2a2b93a37f in gsignal () from /lib64/libc.so.6
#1  0x00007f2a2c220aa1 in sighdlHandler(int) () from /opt/ims/lib/libims_common.so
#2  <signal handler called>
#3  0x00007f2a2c3319ff in DBL_Access_read::readImpiAccessType(std::string const&) () from /opt/ims/lib/libims_hss1_dbl.so
#4  0x00007f2a2c33a8e1 in SLM_CxManagerSAR::storeAccessType() () from /opt/ims/lib/libims_hss1_slm.so
#5  0x00007f2a2b9f2ea5 in start_thread () from /lib64/libpthread.so.0

Thread 2 (Thread 0x7f2a1a2b0700 (LWP 3109)):
#0  0x00007f2a2b9c8f4a in pthread_cond_wait () from /lib64/libpthread.so.0
#1  0x00007f2a2c0aa311 in grpc_core::Executor::ThreadMain() () from /opt/ims/lib/libgrpc.so
)");

    InvestigationCreateRequest createRequest;
    createRequest.name = "crash-sig";

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
    EXPECT_EQ(CrashAnalysisStatus::Ready, crashResult->status);
    ASSERT_TRUE(crashResult->faultThreadId.has_value());
    EXPECT_EQ("6", *crashResult->faultThreadId);
    ASSERT_FALSE(crashResult->observations.empty());
    EXPECT_NE(std::string::npos, crashResult->observations.front().find("DBL_Access_read::readImpiAccessType"));
    EXPECT_EQ(std::string::npos, crashResult->observations.front().find("sighdlHandler"));
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

    ASSERT_TRUE(crashResult.hasValue());
    EXPECT_EQ(CrashAnalysisStatus::NotSupported, crashResult->status);
    EXPECT_EQ("log", crashResult->artifactType);
}
