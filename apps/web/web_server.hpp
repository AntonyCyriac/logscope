/**
 * @file web_server.hpp
 * @brief Embedded HTTP server for logscope-web (M15.1 / C12).
 */

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "analyze_job_queue.hpp"
#include "session_store.hpp"
#include "web_config.hpp"
#include "workspace_store.hpp"

namespace httplib
{
class Server;
} // namespace httplib

namespace scope::web
{

/**
 * @brief REST API server wrapping cpp-httplib.
 */
class WebServer
{
  public:
    explicit WebServer(WebConfig config);
    ~WebServer();

    WebServer(const WebServer&) = delete;
    WebServer& operator=(const WebServer&) = delete;

    /**
     * @brief Blocks until the server stops.
     */
    [[nodiscard]] bool run();

    /**
     * @brief Starts listening on an ephemeral or configured port in a background thread.
     */
    [[nodiscard]] bool startInBackground();

    void stop();

    [[nodiscard]] int port() const noexcept;

    [[nodiscard]] SessionStore& sessionStore() noexcept;

    [[nodiscard]] WorkspaceStore& workspaceStore() noexcept;

    [[nodiscard]] AnalyzeJobQueue& jobQueue() noexcept;

    [[nodiscard]] const WebConfig& config() const noexcept;

  private:
    void registerRoutes();

    WebConfig m_config;
    SessionStore m_sessionStore;
    WorkspaceStore m_workspaceStore;
    AnalyzeJobQueue m_jobQueue;
    std::unique_ptr<httplib::Server> m_server;
    std::thread m_thread;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_stopped{false};
    std::atomic<int> m_port{0};
    const std::chrono::steady_clock::time_point m_startTime{std::chrono::steady_clock::now()};
};

} // namespace scope::web
