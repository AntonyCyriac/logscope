/**
 * @file investigation_controller_test.cpp
 */

#include <fstream>

#include <gtest/gtest.h>

#include "gtest_temp_path.hpp"
#include "investigation_controller.hpp"

using scope::application::InvestigationController;
using scope::foundation::Path;
using scope::workspace::CrashAnalysisStatus;

namespace
{

void writeFile(const Path& path, const std::string& contents)
{
    std::ofstream stream(path.string(), std::ios::binary | std::ios::trunc);
    stream << contents;
}

} // namespace

TEST(InvestigationControllerTest, CreateAddArtifactsTimelineAndCrash)
{
    const Path root(logscope::gtest::uniqueTestPath("_inv_ctrl"));
    const Path logFile(logscope::gtest::uniqueTestPath("_sample.log"));
    const Path pstackFile(logscope::gtest::uniqueTestPath("_pstack.txt"));

    writeFile(logFile,
              "2026-07-11 10:00:01 INFO Application started\n"
              "2026-07-11 10:00:06 ERROR Connection refused\n");

    writeFile(pstackFile, R"(Thread 1 (LWP 12345):
#0  0x00005555555a2b10 in SessionManager::create (this=0x0) at session_manager.cpp:42
#1  0x00005555555a1c01 in main (argc=1, argv=0x7fffffffe5a8) at app.cpp:18

Program received signal SIGSEGV, Segmentation fault.
[Switching to thread 1 (LWP 12345)]
)");

    InvestigationController controller(root);

    const auto createResult = controller.create("desktop-parity-test");

    ASSERT_TRUE(createResult.hasValue());
    EXPECT_FALSE(controller.manifest().id.empty());

    const auto logArtifact = controller.addLogArtifact(logFile, "sample.log");

    ASSERT_TRUE(logArtifact.hasValue());

    const auto pstackArtifact = controller.addArtifactFile(pstackFile, "pstack", "pstack.txt");

    ASSERT_TRUE(pstackArtifact.hasValue());

    const auto timelineResult = controller.projectTimeline();

    ASSERT_TRUE(timelineResult.hasValue());
    EXPECT_GT(timelineResult->events.size(), 0U);

    bool foundCrashSummary = false;

    for (const auto& event : timelineResult->events)
    {
        if (event.eventType == "crash.summary")
        {
            foundCrashSummary = true;
            break;
        }
    }

    EXPECT_TRUE(foundCrashSummary);

    const auto crashResult = controller.analyzeCrash(pstackArtifact->id);

    ASSERT_TRUE(crashResult.hasValue());
    EXPECT_EQ(CrashAnalysisStatus::Ready, crashResult->status);
    EXPECT_TRUE(crashResult->signal.has_value());
    EXPECT_EQ("SIGSEGV", *crashResult->signal);

    const auto textResult = controller.readArtifactText(pstackArtifact->id);

    ASSERT_TRUE(textResult.hasValue());
    EXPECT_NE(std::string::npos, textResult->find("SessionManager::create"));
}
