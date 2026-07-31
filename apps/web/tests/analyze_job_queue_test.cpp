/**
 * @file analyze_job_queue_test.cpp
 * @brief Unit tests for AnalyzeJobQueue (M15.3 / J1).
 */

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "analyze_job_queue.hpp"
#include "analysis_config.hpp"
#include "foundation/path.hpp"
#include "session_store.hpp"
#include "web_config.hpp"

namespace
{

class AnalyzeJobQueueTest : public ::testing::Test
{
  protected:
    scope::web::WebConfig config = scope::web::WebConfig::defaults();
    scope::web::SessionStore sessionStore;
    scope::web::AnalyzeJobQueue queue{config, sessionStore};
};

} // namespace

TEST_F(AnalyzeJobQueueTest, PollUnknownJobReturnsNotFound)
{
    const std::string sessionId = sessionStore.createWorkspace();
    const auto pollResult = queue.poll(sessionId, "00000000-0000-0000-0000-000000000000");

    EXPECT_FALSE(pollResult);
}

TEST_F(AnalyzeJobQueueTest, CrossSessionPollReturnsNotFound)
{
    const std::string sessionA = sessionStore.createWorkspace();
    const std::string sessionB = sessionStore.createWorkspace();

    scope::web::WebConfig tinyThresholdConfig = config;
    tinyThresholdConfig.asyncAnalyzeThresholdBytes = 0U;
    tinyThresholdConfig.bindPort = 0;

    scope::web::AnalyzeJobQueue localQueue(tinyThresholdConfig, sessionStore);

    const std::string samplePath = std::string(LOGSCOPE_SOURCE_DIR) + "/samples/sample.log";
    scope::web::WorkspaceSession* workspace = sessionStore.findSession(sessionA);
    ASSERT_NE(nullptr, workspace);

    {
        std::lock_guard<std::mutex> lock(workspace->mutex);
        ASSERT_TRUE(workspace->service->openSource(scope::foundation::Path(samplePath)));
    }

    scope::analysis::AnalysisConfig analysisConfig;
    const auto enqueueResult = localQueue.enqueue(sessionA, analysisConfig);
    ASSERT_TRUE(enqueueResult);

    const auto crossPoll = localQueue.poll(sessionB, enqueueResult->jobId);
    EXPECT_FALSE(crossPoll);
}
