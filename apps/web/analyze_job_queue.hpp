/**
 * @file analyze_job_queue.hpp
 * @brief Per-session async analyze jobs (M15.3 / C12g).
 */

#pragma once

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include "analysis_config.hpp"
#include "foundation/result.hpp"
#include "session_store.hpp"
#include "web_config.hpp"

namespace scope::web
{

/**
 * @brief Async analyze job lifecycle state.
 */
enum class AnalyzeJobStatus
{
    Running,
    Completed,
    Failed
};

/**
 * @brief Enqueue response for 202 Accepted.
 */
struct AnalyzeJobEnqueueResult
{
    std::string jobId;
    std::string pollUrl;
};

/**
 * @brief Session-scoped async analyze job queue.
 */
class AnalyzeJobQueue
{
  public:
    AnalyzeJobQueue(const WebConfig& config, SessionStore& sessionStore);

    [[nodiscard]] foundation::Result<AnalyzeJobEnqueueResult> enqueue(const std::string& sessionId,
                                                                      const analysis::AnalysisConfig& config);

    /**
     * @brief Polls job status; returns JSON object (without envelope) for the data field.
     */
    [[nodiscard]] foundation::Result<std::string> poll(const std::string& sessionId, const std::string& jobId);

    void evictExpired();

  private:
    struct JobRecord
    {
        std::string sessionId;
        AnalyzeJobStatus status = AnalyzeJobStatus::Running;
        std::string resultJson;
        std::string errorCode;
        std::string errorMessage;
        std::chrono::steady_clock::time_point createdAt{std::chrono::steady_clock::now()};
        std::chrono::steady_clock::time_point finishedAt{};
    };

    [[nodiscard]] bool hasRunningJobForSession(const std::string& sessionId) const;

    const WebConfig& m_config;
    SessionStore& m_sessionStore;
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, JobRecord> m_jobs;
};

} // namespace scope::web
