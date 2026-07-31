/**
 * @file analyze_job_queue_test.cpp
 * @brief Unit tests for AnalyzeJobQueue (M15.3 / J1).
 */

#include <gtest/gtest.h>

#include "analyze_job_queue.hpp"
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
    const std::string jobId = "11111111-1111-1111-1111-111111111111";

    queue.seedJobForTest(sessionA, jobId, scope::web::AnalyzeJobStatus::Completed);

    const auto crossPoll = queue.poll(sessionB, jobId);
    EXPECT_FALSE(crossPoll);

    const auto ownerPoll = queue.poll(sessionA, jobId);
    EXPECT_TRUE(ownerPoll);
}
