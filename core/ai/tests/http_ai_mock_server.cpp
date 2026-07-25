/**
 * @file http_ai_mock_server.cpp
 */

#include "http_ai_mock_server.hpp"

#include <httplib.h>

namespace scope::ai::test
{

HttpAiMockServer::HttpAiMockServer() : m_server(new httplib::Server()) {}

HttpAiMockServer::~HttpAiMockServer()
{
    stop();
    delete m_server;
}

void HttpAiMockServer::setResponse(std::string responseBody)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_responseBody = std::move(responseBody);
}

void HttpAiMockServer::setDelay(const std::chrono::milliseconds delay)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_delay = delay;
}

void HttpAiMockServer::start()
{
    m_server->Post("/v1/chat/completions",
                   [this](const httplib::Request& request, httplib::Response& response) {
                       handleChatCompletions(request, response);
                   });

    m_port = m_server->bind_to_any_port("127.0.0.1");

    m_thread = std::thread([this]() { m_server->listen_after_bind(); });
}

void HttpAiMockServer::stop()
{
    if (m_port != 0)
    {
        m_server->stop();
    }

    if (m_thread.joinable())
    {
        m_thread.join();
    }

    m_port = 0;
}

std::string HttpAiMockServer::endpoint() const
{
    return "http://127.0.0.1:" + std::to_string(m_port) + "/v1";
}

std::string HttpAiMockServer::lastRequestBody() const
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    return m_lastRequestBody;
}

void HttpAiMockServer::handleChatCompletions(const httplib::Request& request, httplib::Response& response)
{
    std::string responseBody;
    std::chrono::milliseconds delay;

    {
        const std::lock_guard<std::mutex> lock(m_mutex);
        m_lastRequestBody = request.body;
        responseBody = m_responseBody;
        delay = m_delay;
    }

    if (delay.count() > 0)
    {
        std::this_thread::sleep_for(delay);
    }

    response.status = 200;
    response.set_content(responseBody, "application/json");
}

} // namespace scope::ai::test
