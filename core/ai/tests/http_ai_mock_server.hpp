/**
 * @file http_ai_mock_server.hpp
 * @brief Local OpenAI-compatible mock server for HTTP AI tests (M13).
 */

#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <thread>

namespace httplib
{
class Server;
class Request;
class Response;
}

namespace scope::ai::test
{

/**
 * @brief Serves canned /v1/chat/completions responses on 127.0.0.1.
 */
class HttpAiMockServer
{
  public:
    HttpAiMockServer();
    ~HttpAiMockServer();

    HttpAiMockServer(const HttpAiMockServer&) = delete;
    HttpAiMockServer& operator=(const HttpAiMockServer&) = delete;

    void setResponse(std::string responseBody);

    void setDelay(std::chrono::milliseconds delay);

    void start();

    void stop();

    [[nodiscard]] std::string endpoint() const;

    [[nodiscard]] std::string lastRequestBody() const;

  private:
    void handleChatCompletions(const httplib::Request& request, httplib::Response& response);

    httplib::Server* m_server;
    std::thread m_thread;
    int m_port{0};
    std::string m_responseBody;
    std::chrono::milliseconds m_delay{0};
    mutable std::mutex m_mutex;
    std::string m_lastRequestBody;
};

} // namespace scope::ai::test
