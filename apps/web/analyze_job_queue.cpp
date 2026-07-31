/**
 * @file analyze_job_queue.cpp
 */

#include "analyze_job_queue.hpp"

#include "rest_json.hpp"

#include "foundation/filesystem.hpp"
#include "foundation/uuid.hpp"

#include <sstream>
#include <thread>

namespace scope::web
{

namespace
{

std::string statusName(const AnalyzeJobStatus status)
{
    switch (status)
    {
    case AnalyzeJobStatus::Running:
        return "running";
    case AnalyzeJobStatus::Completed:
        return "completed";
    case AnalyzeJobStatus::Failed:
        return "failed";
    }

    return "unknown";
}

} // namespace

AnalyzeJobQueue::AnalyzeJobQueue(const WebConfig& config, SessionStore& sessionStore)
    : m_config(config)
    , m_sessionStore(sessionStore)
{
}

AnalyzeJobQueue::~AnalyzeJobQueue()
{
    while (m_activeWorkers.load(std::memory_order_acquire) > 0U)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

bool AnalyzeJobQueue::hasRunningJobForSession(const std::string& sessionId) const
{
    for (const auto& entry : m_jobs)
    {
        if (entry.second.sessionId == sessionId && entry.second.status == AnalyzeJobStatus::Running)
        {
            return true;
        }
    }

    return false;
}

void AnalyzeJobQueue::evictExpired()
{
    const auto now = std::chrono::steady_clock::now();
    const auto ttl = std::chrono::seconds(m_config.jobTtlSeconds);

    for (auto iterator = m_jobs.begin(); iterator != m_jobs.end();)
    {
        const bool expired = iterator->second.status != AnalyzeJobStatus::Running &&
                             (now - iterator->second.finishedAt) > ttl;

        if (expired)
        {
            iterator = m_jobs.erase(iterator);
        }
        else
        {
            ++iterator;
        }
    }
}

foundation::Result<AnalyzeJobEnqueueResult> AnalyzeJobQueue::enqueue(const std::string& sessionId,
                                                                     const analysis::AnalysisConfig& config)
{
    WorkspaceSession* workspace = m_sessionStore.findSession(sessionId);

    if (workspace == nullptr)
    {
        return foundation::Result<AnalyzeJobEnqueueResult>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Unknown session."));
    }

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        evictExpired();

        if (m_config.jobMaxConcurrentPerSession > 0 && hasRunningJobForSession(sessionId))
        {
            return foundation::Result<AnalyzeJobEnqueueResult>(foundation::Error(
                foundation::ErrorCode::InvalidArgument, "An analyze job is already running for this session."));
        }
    }

    const std::string jobId = foundation::Uuid::generate().toString();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        JobRecord record;
        record.sessionId = sessionId;
        record.status = AnalyzeJobStatus::Running;
        record.createdAt = std::chrono::steady_clock::now();
        m_jobs.emplace(jobId, std::move(record));
    }

    std::thread([this, jobId, sessionId, config]() {
        m_activeWorkers.fetch_add(1U, std::memory_order_release);
        const auto releaseWorker = [this]() { m_activeWorkers.fetch_sub(1U, std::memory_order_release); };

        WorkspaceSession* workspaceSession = m_sessionStore.findSession(sessionId);

        if (workspaceSession == nullptr)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            const auto iterator = m_jobs.find(jobId);

            if (iterator != m_jobs.end())
            {
                JobRecord& record = iterator->second;
                record.status = AnalyzeJobStatus::Failed;
                record.errorCode = "INVALID_STATE";
                record.errorMessage = "Session no longer exists.";
                record.finishedAt = std::chrono::steady_clock::now();
            }

            releaseWorker();

            return;
        }

        foundation::Result<analysis::AnalysisModel> analyzeResult = [&]() {
            std::lock_guard<std::mutex> sessionLock(workspaceSession->mutex);
            return workspaceSession->service->analyze(config);
        }();

        std::lock_guard<std::mutex> lock(m_mutex);
        const auto iterator = m_jobs.find(jobId);

        if (iterator == m_jobs.end())
        {
            releaseWorker();

            return;
        }

        JobRecord& record = iterator->second;
        record.finishedAt = std::chrono::steady_clock::now();

        if (!analyzeResult)
        {
            record.status = AnalyzeJobStatus::Failed;
            record.errorCode = "INTERNAL";
            record.errorMessage = analyzeResult.error().message();
            releaseWorker();

            return;
        }

        record.status = AnalyzeJobStatus::Completed;
        record.resultJson = formatAnalyzeJson(*analyzeResult);
        releaseWorker();
    }).detach();

    AnalyzeJobEnqueueResult result;
    result.jobId = jobId;
    result.pollUrl = "/api/v1/jobs/" + jobId;

    return foundation::Result<AnalyzeJobEnqueueResult>(std::move(result));
}

foundation::Result<std::string> AnalyzeJobQueue::poll(const std::string& sessionId, const std::string& jobId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    evictExpired();

    const auto iterator = m_jobs.find(jobId);

    if (iterator == m_jobs.end())
    {
        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Job not found."));
    }

    const JobRecord& record = iterator->second;

    if (record.sessionId != sessionId)
    {
        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::FileNotFound, "Job not found."));
    }

    std::ostringstream output;
    output << "{\n"
           << "  \"jobId\": \"" << escapeJsonString(jobId) << "\",\n"
           << "  \"status\": \"" << statusName(record.status) << "\"";

    if (record.status == AnalyzeJobStatus::Completed)
    {
        output << ",\n  \"result\": " << record.resultJson;
    }
    else if (record.status == AnalyzeJobStatus::Failed)
    {
        output << ",\n  \"error\": {\n"
               << "    \"code\": \"" << escapeJsonString(record.errorCode) << "\",\n"
               << "    \"message\": \"" << escapeJsonString(record.errorMessage) << "\"\n"
               << "  }";
    }

    output << "\n}";

    return foundation::Result<std::string>(output.str());
}

} // namespace scope::web
