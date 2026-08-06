/**
 * @file web_server.cpp
 */

#include "web_server.hpp"

#include "artifact_handler.hpp"
#include "investigation_container.hpp"
#include "json_parse.hpp"
#include "middleware/api_key.hpp"
#include "rest_json.hpp"
#include "session_resource_cleanup.hpp"
#include "web_request_parsers.hpp"
#include "workspace.hpp"

#include "foundation/error.hpp"
#include "foundation/filesystem.hpp"
#include "foundation/uuid.hpp"

#include <httplib.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <algorithm>
#include <iostream>
#include <sstream>

#ifndef LOGSCOPE_WEB_UI_DIR
#define LOGSCOPE_WEB_UI_DIR "apps/web/ui/dist"
#endif

namespace scope::web
{

namespace
{

std::string uptimeSeconds(const std::chrono::steady_clock::time_point startTime)
{
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - startTime);

    return std::to_string(elapsed.count());
}

std::filesystem::path resolveUiDirectory()
{
    if (const char* envPath = std::getenv("LOGSCOPE_WEB_UI_DIR"))
    {
        const std::filesystem::path candidate(envPath);

        if (std::filesystem::is_directory(candidate))
        {
            return candidate;
        }
    }

    const std::filesystem::path compileTime(LOGSCOPE_WEB_UI_DIR);

    if (std::filesystem::is_directory(compileTime))
    {
        return compileTime;
    }

    const std::filesystem::path bundleUi = std::filesystem::current_path() / "ui" / "dist";

    if (std::filesystem::is_directory(bundleUi))
    {
        return bundleUi;
    }

    const std::filesystem::path devUi = std::filesystem::current_path() / "apps" / "web" / "ui" / "dist";

    if (std::filesystem::is_directory(devUi))
    {
        return devUi;
    }

    return compileTime;
}

void setJsonResponse(httplib::Response& response, const int status, const std::string& body)
{
    response.status = status;
    response.set_content(body, "application/json; charset=utf-8");
}

void setErrorResponse(httplib::Response& response, const foundation::Error& error)
{
    setJsonResponse(response, httpStatusForError(error), errorEnvelopeFromFoundation(error));
}

WorkspaceSession* requireSession(WebServer& server, const std::string& sessionId, httplib::Response& response)
{
    WorkspaceSession* workspace = server.sessionStore().findSession(sessionId);

    if (workspace == nullptr)
    {
        setJsonResponse(response, 400,
                        errorEnvelope("INVALID_ARGUMENT", "Unknown or missing session. Create one via POST /api/v1/sessions/workspace."));

        return nullptr;
    }

    return workspace;
}

void warnIfExposedWithoutApiKey(const WebConfig& config)
{
    if (!config.apiKey.empty() || WebConfig::isLoopbackBindHost(config.bindHost))
    {
        return;
    }

    std::cerr << "logscope-web: WARNING: bind_host is not loopback and web.api_key is empty — API is exposed "
                 "without authentication. Set web.api_key or use a reverse proxy with TLS."
              << std::endl;
}

void warnIfPlaintextApiKeyInConfig(const WebConfig& config)
{
    if (!config.apiKey.isPlaintextInConfig())
    {
        return;
    }

    std::cerr << "logscope-web: WARNING: web.api_key is stored in plain text. Prefer web.api_key_hash "
                 "(generate with: logscope-web --hash-api-key <secret>)."
              << std::endl;
}

bool rejectStaleSessionHeader(const httplib::Request& request, const std::string& resolvedSessionId,
                              httplib::Response& response)
{
    const auto iterator = request.headers.find(kSessionHeader);

    if (iterator != request.headers.end() && !iterator->second.empty() && resolvedSessionId.empty())
    {
        setJsonResponse(response, 401, errorEnvelope("SESSION_EXPIRED", "Session expired or invalid."));

        return true;
    }

    return false;
}

void sweepIdleSessions(WebServer& server)
{
    const WebConfig& config = server.config();

    if (config.sessionTtlSeconds <= 0)
    {
        return;
    }

    (void)server.sessionStore().evictIdleSessions(
        std::chrono::seconds(config.sessionTtlSeconds),
        [&server](const std::string& sessionId) { return server.jobQueue().hasRunningJobForSession(sessionId); });
}

std::function<bool(const std::string&)> skipSessionsWithRunningJobs(WebServer& server)
{
    return [&server](const std::string& sessionId) { return server.jobQueue().hasRunningJobForSession(sessionId); };
}

} // namespace

WebServer::WebServer(WebConfig config)
    : m_config(std::move(config))
    , m_workspaceStore(m_config)
    , m_jobQueue(m_config, m_sessionStore)
{
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    if (m_config.tlsEnabled())
    {
        m_server = std::make_unique<httplib::SSLServer>(m_config.tlsCertPath.string().c_str(),
                                                        m_config.tlsKeyPath.string().c_str());
    }
    else
    {
        m_server = std::make_unique<httplib::Server>();
    }
#else
    if (m_config.tlsEnabled())
    {
        std::cerr << "TLS is configured but logscope-web was built without OpenSSL support." << std::endl;

        return;
    }

    m_server = std::make_unique<httplib::Server>();
#endif

    if (m_server == nullptr)
    {
        return;
    }

    m_server->set_read_timeout(m_config.requestTimeoutSeconds, 0);
    m_server->set_write_timeout(m_config.requestTimeoutSeconds, 0);
    registerRoutes();
}

WebServer::~WebServer()
{
    stop();
}

bool WebServer::run()
{
    if (m_server == nullptr)
    {
        return false;
    }

    m_running = true;
    m_port = m_config.bindPort;
    warnIfExposedWithoutApiKey(m_config);
    warnIfPlaintextApiKeyInConfig(m_config);

    return m_server->listen(m_config.bindHost.c_str(), m_config.bindPort);
}

bool WebServer::startInBackground()
{
    if (m_running || m_server == nullptr)
    {
        return false;
    }

    int boundPort = -1;

    if (m_config.bindPort == 0)
    {
        boundPort = m_server->bind_to_any_port(m_config.bindHost.c_str());
    }
    else
    {
        boundPort = m_server->bind_to_port(m_config.bindHost.c_str(), m_config.bindPort);
    }

    if (boundPort < 0)
    {
        return false;
    }

    m_port = boundPort;
    m_running = true;
    warnIfExposedWithoutApiKey(m_config);
    warnIfPlaintextApiKeyInConfig(m_config);
    m_thread = std::thread([this]() { m_server->listen_after_bind(); });

    return true;
}

void WebServer::stop()
{
    if (m_stopped.exchange(true))
    {
        return;
    }

    if (m_running)
    {
        m_server->stop();

        if (m_thread.joinable())
        {
            m_thread.join();
        }

        m_running = false;
    }

    m_sessionStore.cleanupAllSessionResources();
    m_jobQueue.waitForIdle(std::chrono::seconds(30));
}

int WebServer::port() const noexcept
{
    return m_port.load();
}

SessionStore& WebServer::sessionStore() noexcept
{
    return m_sessionStore;
}

WorkspaceStore& WebServer::workspaceStore() noexcept
{
    return m_workspaceStore;
}

AnalyzeJobQueue& WebServer::jobQueue() noexcept
{
    return m_jobQueue;
}

const WebConfig& WebServer::config() const noexcept
{
    return m_config;
}

namespace
{

void applyCors(const scope::web::WebConfig& config, const httplib::Request& request, httplib::Response& response)
{
    if (config.corsOrigins.empty())
    {
        return;
    }

    const std::string origin = request.get_header_value("Origin");
    bool originAllowed = false;

    if (!origin.empty())
    {
        for (const std::string& allowed : config.corsOrigins)
        {
            if (allowed == origin)
            {
                response.set_header("Access-Control-Allow-Origin", origin);
                originAllowed = true;
                break;
            }
        }
    }

    if (!originAllowed)
    {
        response.set_header("Access-Control-Allow-Origin", config.corsOrigins.front());
    }

    response.set_header("Access-Control-Allow-Headers", "Content-Type, X-LogScope-Session, X-LogScope-Api-Key");
    response.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
}

std::string resolveSessionId(scope::web::SessionStore& sessionStore, const httplib::Request& request,
                             const bool autoCreate)
{
    const auto iterator = request.headers.find(scope::web::kSessionHeader);
    const std::string existing = iterator != request.headers.end() ? iterator->second : std::string();

    return sessionStore.resolveSession(existing, autoCreate);
}

} // namespace

void WebServer::registerRoutes()
{
    m_server->Options(R"(/.*)", [this](const httplib::Request& request, httplib::Response& response) {
        applyCors(m_config, request, response);
        response.status = 204;
    });

    m_server->Get("/api/v1/health", [this](const httplib::Request& request, httplib::Response& response) {
        applyCors(m_config, request, response);

        if (m_config.healthRequiresApiKey && !m_config.apiKey.empty())
        {
            if (!authorizeApiKey(m_config.apiKey, request, response))
            {
                return;
            }
        }

        sweepIdleSessions(*this);

        std::ostringstream body;
        body << "{\n"
             << "  \"version\": \"" << escapeJsonString(LOGSCOPE_VERSION) << "\",\n"
             << "  \"uptimeSeconds\": " << uptimeSeconds(m_startTime) << ",\n"
             << "  \"sessionCount\": " << m_sessionStore.sessionCount() << "\n"
             << '}';

        setJsonResponse(response, 200, successEnvelope(body.str()));
    });

    m_server->Post("/api/v1/sessions/workspace", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        sweepIdleSessions(*this);

        if (m_config.maxSessions > 0)
        {
            const auto skipSession = skipSessionsWithRunningJobs(*this);

            while (m_sessionStore.sessionCount() >= static_cast<std::size_t>(m_config.maxSessions))
            {
                if (m_sessionStore.evictSessionsForCapacity(1U, skipSession) == 0U)
                {
                    break;
                }
            }

            if (m_sessionStore.sessionCount() >= static_cast<std::size_t>(m_config.maxSessions))
            {
                setJsonResponse(response, 503,
                                errorEnvelope("SERVICE_UNAVAILABLE", "Maximum session count reached."));

                return;
            }
        }

        const std::string sessionId = m_sessionStore.createWorkspace();
        setJsonResponse(response, 200, successEnvelope("{\"sessionId\": \"" + escapeJsonString(sessionId) + "\"}"));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/config/load", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        const auto pathResult = parsePathField(request.body);

        if (!pathResult)
        {
            setErrorResponse(response, pathResult.error());

            return;
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);
        const auto loadResult = workspace->service->loadConfiguration(pathResult.value());

        if (!loadResult)
        {
            setErrorResponse(response, loadResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope("{\"loaded\": true}"));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/config/validate", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        std::vector<std::string> requiredKeys;

        if (const std::optional<std::string> required = jsonStringField(request.body, "requiredKeys"))
        {
            requiredKeys.push_back(*required);
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);
        const auto validateResult = workspace->service->validateConfiguration(requiredKeys);

        if (!validateResult)
        {
            setErrorResponse(response, validateResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope("{\"valid\": true}"));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/sources/open", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        if (!m_config.allowServerPaths)
        {
            setJsonResponse(response, 403,
                            errorEnvelope("FORBIDDEN", "Server path open is disabled (web.allow_server_paths=false)."));

            return;
        }

        const auto pathResult = parsePathField(request.body);

        if (!pathResult)
        {
            setErrorResponse(response, pathResult.error());

            return;
        }

        const auto pathValidation = validateServerPath(m_config, pathResult.value());

        if (!pathValidation)
        {
            setErrorResponse(response, pathValidation.error());

            return;
        }

        const foundation::Path sourcePath = pathResult.value();

        {
            std::lock_guard<std::mutex> lock(workspace->mutex);
            removeTempUploadFile(*workspace);
        }

        const auto openResult = workspace->service->openSource(sourcePath);

        if (!openResult)
        {
            setErrorResponse(response, openResult.error());

            return;
        }

        setJsonResponse(response, 200,
                        successEnvelope("{\"sourcePath\": \"" + escapeJsonString(sourcePath.string()) + "\"}"));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/sources/upload", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        if (!request.is_multipart_form_data())
        {
            setJsonResponse(response, 400, errorEnvelope("INVALID_ARGUMENT", "Expected multipart/form-data upload."));

            return;
        }

        const auto fileIterator = request.files.find("file");

        if (fileIterator == request.files.end())
        {
            setJsonResponse(response, 400, errorEnvelope("INVALID_ARGUMENT", "Missing multipart field: file"));

            return;
        }

        const httplib::MultipartFormData& file = fileIterator->second;

        if (static_cast<std::uint64_t>(file.content.size()) > m_config.maxUploadBytes)
        {
            setJsonResponse(response, 413, errorEnvelope("PAYLOAD_TOO_LARGE", "Upload exceeds web.max_upload_bytes"));

            return;
        }

        const foundation::Path tempDir =
            m_config.uploadTempDir.string().empty() ? foundation::Path(std::filesystem::temp_directory_path().string())
                                                    : m_config.uploadTempDir;
        const foundation::Path tempFile = foundation::Path(tempDir.string() + "/logscope-upload-" +
                                                             foundation::Uuid::generate().toString() + ".log");

        {
            std::ofstream stream(tempFile.string(), std::ios::binary);

            if (!stream)
            {
                setJsonResponse(response, 500, errorEnvelope("INTERNAL", "Failed to stage upload."));

                return;
            }

            stream.write(file.content.data(), static_cast<std::streamsize>(file.content.size()));
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);
        replaceStagedUpload(*workspace, tempFile.string());
        const auto openResult = workspace->service->openSource(tempFile);

        if (!openResult)
        {
            setErrorResponse(response, openResult.error());

            return;
        }

        setJsonResponse(response, 200,
                        successEnvelope("{\"sourcePath\": \"" + escapeJsonString(tempFile.string()) + "\"}"));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/analyze", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        const analysis::AnalysisConfig analysisConfig =
            parseAnalysisConfig(request.body, workspace->service->configurationManager());

        std::uint64_t sourceSize = 0U;
        bool hasSource = false;

        {
            std::lock_guard<std::mutex> lock(workspace->mutex);

            if (workspace->service->sourcePath().string().empty())
            {
                setJsonResponse(response, 409, errorEnvelope("INVALID_STATE", "Open a source before analyze."));

                return;
            }

            const auto sizeResult = foundation::FileSystem::fileSize(workspace->service->sourcePath());

            if (!sizeResult)
            {
                setErrorResponse(response, sizeResult.error());

                return;
            }

            sourceSize = *sizeResult;
            hasSource = true;
        }

        if (!hasSource)
        {
            return;
        }

        if (sourceSize >= m_config.asyncAnalyzeThresholdBytes)
        {
            const auto enqueueResult = m_jobQueue.enqueue(sessionId, analysisConfig);

            if (!enqueueResult)
            {
                if (enqueueResult.error().message().find("already running") != std::string::npos)
                {
                    setJsonResponse(response, 409,
                                    errorEnvelope("INVALID_STATE", enqueueResult.error().message()));
                }
                else
                {
                    setErrorResponse(response, enqueueResult.error());
                }

                return;
            }

            const std::string location = "/api/v1/jobs/" + enqueueResult->jobId;
            setJsonResponse(response, 202, successEnvelope(formatAnalyzeJobAccepted(*enqueueResult)));
            response.set_header("Location", location);
            response.set_header(kSessionHeader, sessionId);

            return;
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);
        const auto analyzeResult = workspace->service->analyze(analysisConfig);

        if (!analyzeResult)
        {
            setErrorResponse(response, analyzeResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(formatAnalyzeJson(*analyzeResult)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/investigate", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        investigation::InvestigationCriteria criteria = parseInvestigationCriteria(request.body);

        std::lock_guard<std::mutex> lock(workspace->mutex);
        scope::investigation::applyInvestigationConfiguration(criteria,
                                                              workspace->service->configurationManager().configuration());

        if (!workspace->service->hasModel())
        {
            setJsonResponse(response, 409, errorEnvelope("INVALID_STATE", "Analyze a source before investigate."));

            return;
        }

        const auto investigateResult = workspace->service->investigate(criteria);

        if (!investigateResult)
        {
            setErrorResponse(response, investigateResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(formatInvestigationJson(*investigateResult)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/analytics", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        const analytics::AnalyticsConfig analyticsConfig = parseAnalyticsConfig(request.body);

        std::lock_guard<std::mutex> lock(workspace->mutex);

        if (!workspace->service->hasModel())
        {
            setJsonResponse(response, 409, errorEnvelope("INVALID_STATE", "Analyze a source before analytics."));

            return;
        }

        const auto analyticsResult = workspace->service->runAnalytics(analyticsConfig);

        if (!analyticsResult)
        {
            setErrorResponse(response, analyticsResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(formatAnalyticsJson(*analyticsResult)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Get("/api/v1/sessions", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        const auto directoryIterator = request.params.find("directory");
        foundation::Path directory;

        if (directoryIterator != request.params.end())
        {
            directory = foundation::Path(directoryIterator->second);
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);
        const auto sessionsResult = workspace->service->listSessions(directory);

        if (!sessionsResult)
        {
            setErrorResponse(response, sessionsResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(formatPathList(*sessionsResult)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/sessions/save", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        application::SessionSaveRequest saveRequest = parseSessionSaveRequest(request.body);
        const std::optional<std::string> investigationId = jsonStringField(request.body, "investigationId");
        const std::optional<std::string> workspaceId = jsonStringField(request.body, "workspaceId");
        const std::optional<std::string> containerId =
            investigationId.has_value() ? investigationId : workspaceId;

        if (containerId.has_value())
        {
            const auto snapshotPath = m_workspaceStore.investigationStore().snapshotPathFor(*containerId);

            if (!snapshotPath)
            {
                setErrorResponse(response, snapshotPath.error());

                return;
            }

            if (saveRequest.sessionFile.string().empty())
            {
                saveRequest.sessionFile = *snapshotPath;
            }
        }

        if (saveRequest.sessionFile.string().empty())
        {
            setJsonResponse(response, 400,
                            errorEnvelope("INVALID_ARGUMENT",
                                          "Missing required field: path, investigationId, or workspaceId."));

            return;
        }

        if (saveRequest.configFile.string().empty())
        {
            saveRequest.configFile = workspace->service->configFilePath();
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);
        const auto saveResult = workspace->service->saveSession(saveRequest);

        if (!saveResult)
        {
            setErrorResponse(response, saveResult.error());

            return;
        }

        if (containerId.has_value())
        {
            m_workspaceStore.investigationStore().updateSummaryFromService(*containerId, *workspace->service);
        }

        setJsonResponse(response, 200, successEnvelope("{\"saved\": true}"));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/sessions/load", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        const auto pathResult = parsePathField(request.body);

        if (!pathResult)
        {
            setErrorResponse(response, pathResult.error());

            return;
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);
        const auto loadResult = workspace->service->loadSession(pathResult.value());

        if (!loadResult)
        {
            setErrorResponse(response, loadResult.error());

            return;
        }

        removeTempUploadFile(*workspace);
        workspace->service->adoptModel(loadResult->analysisModel(), loadResult->sourcePath());

        if (!loadResult->configFile().string().empty())
        {
            const auto configLoadResult = workspace->service->loadConfiguration(loadResult->configFile());

            if (!configLoadResult)
            {
                setErrorResponse(response, configLoadResult.error());

                return;
            }
        }

        std::ostringstream data;
        data << "{\n"
             << "  \"sessionId\": \"" << escapeJsonString(loadResult->sessionId().toString()) << "\",\n"
             << "  \"sourcePath\": \"" << escapeJsonString(loadResult->sourcePath().string()) << "\"\n"
             << '}';

        setJsonResponse(response, 200, successEnvelope(data.str()));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/workspaces", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        const WorkspaceCreateRequest createRequest = parseWorkspaceCreateRequest(request.body);
        auto createResult = m_workspaceStore.create(createRequest);

        if (!createResult)
        {
            setErrorResponse(response, createResult.error());

            return;
        }

        WorkspaceMetadata metadata = std::move(*createResult);

        if (createRequest.captureSession)
        {
            const auto snapshotPath = m_workspaceStore.snapshotPathFor(metadata.id);

            if (!snapshotPath)
            {
                setErrorResponse(response, snapshotPath.error());

                return;
            }

            std::lock_guard<std::mutex> lock(workspace->mutex);

            if (workspace->service->hasModel())
            {
                application::SessionSaveRequest saveRequest;
                saveRequest.sessionFile = *snapshotPath;
                saveRequest.configFile = workspace->service->configFilePath();
                const auto saveResult = workspace->service->saveSession(saveRequest);

                if (!saveResult)
                {
                    setErrorResponse(response, saveResult.error());

                    return;
                }

                metadata.summary.hasModel = true;
                metadata.summary.lineCount = workspace->service->model().totalLines();
                metadata.summary.errorCount = workspace->service->model().levelCounts().errorLines();
            }
        }

        m_workspaceStore.updateSummaryFromService(metadata.id, *workspace->service);
        const auto refreshed = m_workspaceStore.getMetadata(metadata.id);

        if (refreshed)
        {
            metadata = *refreshed;
        }

        setJsonResponse(response, 200, successEnvelope(formatWorkspaceMetadata(metadata)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Get("/api/v1/workspaces", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        if (requireSession(*this, sessionId, response) == nullptr)
        {
            return;
        }

        const auto listResult = m_workspaceStore.list(m_config.workspacesListLimit);

        if (!listResult)
        {
            setErrorResponse(response, listResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(formatWorkspaceList(*listResult)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Get(R"(/api/v1/workspaces/([^/]+))", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        if (requireSession(*this, sessionId, response) == nullptr)
        {
            return;
        }

        const std::string workspaceId = request.matches[1];
        const auto metadataResult = m_workspaceStore.getMetadata(workspaceId);

        if (!metadataResult)
        {
            setErrorResponse(response, metadataResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(formatWorkspaceMetadata(*metadataResult)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Put(R"(/api/v1/workspaces/([^/]+))", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        if (requireSession(*this, sessionId, response) == nullptr)
        {
            return;
        }

        const std::string workspaceId = request.matches[1];
        const WorkspaceUpdateRequest updateRequest = parseWorkspaceUpdateRequest(request.body);
        const auto updateResult = m_workspaceStore.updateMetadata(workspaceId, updateRequest);

        if (!updateResult)
        {
            setErrorResponse(response, updateResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(formatWorkspaceMetadata(*updateResult)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Delete(R"(/api/v1/workspaces/([^/]+))",
                     [this](const httplib::Request& request, httplib::Response& response) {
                         if (!authorizeApiKey(m_config.apiKey, request, response))
                         {
                             return;
                         }

                         applyCors(m_config, request, response);

                         const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                         if (rejectStaleSessionHeader(request, sessionId, response))
                         {
                             return;
                         }

                         if (requireSession(*this, sessionId, response) == nullptr)
                         {
                             return;
                         }

                         const std::string workspaceId = request.matches[1];
                         const auto removeResult = m_workspaceStore.remove(workspaceId);

                         if (!removeResult)
                         {
                             setErrorResponse(response, removeResult.error());

                             return;
                         }

                         setJsonResponse(response, 200, successEnvelope("{\"deleted\": true}"));
                         response.set_header(kSessionHeader, sessionId);
                     });

    m_server->Post(R"(/api/v1/workspaces/([^/]+)/open)",
                   [this](const httplib::Request& request, httplib::Response& response) {
                       if (!authorizeApiKey(m_config.apiKey, request, response))
                       {
                           return;
                       }

                       applyCors(m_config, request, response);

                       const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                       if (rejectStaleSessionHeader(request, sessionId, response))
                       {
                           return;
                       }

                       WorkspaceSession* workspace = requireSession(*this, sessionId, response);

                       if (workspace == nullptr)
                       {
                           return;
                       }

                       const std::string workspaceId = request.matches[1];
                       const auto snapshotPath = m_workspaceStore.resolveSnapshotPath(workspaceId);

                       if (!snapshotPath)
                       {
                           setErrorResponse(response, snapshotPath.error());

                           return;
                       }

                       std::lock_guard<std::mutex> lock(workspace->mutex);
                       const auto loadResult = workspace->service->loadSession(*snapshotPath);

                       if (!loadResult)
                       {
                           setErrorResponse(response, loadResult.error());

                           return;
                       }

                       removeTempUploadFile(*workspace);
                       workspace->service->adoptModel(loadResult->analysisModel(), loadResult->sourcePath());

                       if (!loadResult->configFile().string().empty())
                       {
                           const auto configLoadResult = workspace->service->loadConfiguration(loadResult->configFile());

                           if (!configLoadResult)
                           {
                               setErrorResponse(response, configLoadResult.error());

                               return;
                           }
                       }

                       m_workspaceStore.touchUpdatedAt(workspaceId);

                       WorkspaceSummary summary;
                       summary.hasModel = workspace->service->hasModel();

                       if (workspace->service->hasModel())
                       {
                           summary.lineCount = workspace->service->model().totalLines();
                           summary.errorCount = workspace->service->model().levelCounts().errorLines();
                       }

                       setJsonResponse(response, 200,
                                       successEnvelope(formatWorkspaceOpenResult(
                                           workspaceId, workspace->service->sourcePath(), summary)));
                       response.set_header(kSessionHeader, sessionId);
                   });

    m_server->Post("/api/v1/investigations", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        const InvestigationCreateBody createBody = parseInvestigationCreateRequest(request.body);
        auto createResult = m_workspaceStore.investigationStore().create(createBody.name, createBody.description);

        if (!createResult)
        {
            setErrorResponse(response, createResult.error());

            return;
        }

        scope::workspace::InvestigationManifest manifest = std::move(*createResult);

        if (createBody.captureSession)
        {
            const auto snapshotPath =
                m_workspaceStore.investigationStore().snapshotPathFor(manifest.id);

            if (!snapshotPath)
            {
                setErrorResponse(response, snapshotPath.error());

                return;
            }

            std::lock_guard<std::mutex> lock(workspace->mutex);

            if (workspace->service->hasModel())
            {
                application::SessionSaveRequest saveRequest;
                saveRequest.sessionFile = *snapshotPath;
                saveRequest.configFile = workspace->service->configFilePath();
                const auto saveResult = workspace->service->saveSession(saveRequest);

                if (!saveResult)
                {
                    setErrorResponse(response, saveResult.error());

                    return;
                }
            }
        }

        m_workspaceStore.investigationStore().updateSummaryFromService(manifest.id, *workspace->service);
        const auto refreshed = m_workspaceStore.investigationStore().get(manifest.id);

        if (refreshed)
        {
            manifest = *refreshed;
        }

        setJsonResponse(response, 200, successEnvelope(formatInvestigationManifest(manifest)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Get("/api/v1/investigations", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        if (requireSession(*this, sessionId, response) == nullptr)
        {
            return;
        }

        const auto listResult = m_workspaceStore.investigationStore().list(m_config.workspacesListLimit);

        if (!listResult)
        {
            setErrorResponse(response, listResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(formatInvestigationList(*listResult)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Get(R"(/api/v1/investigations/([^/]+))",
                  [this](const httplib::Request& request, httplib::Response& response) {
                      if (!authorizeApiKey(m_config.apiKey, request, response))
                      {
                          return;
                      }

                      applyCors(m_config, request, response);

                      const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                      if (rejectStaleSessionHeader(request, sessionId, response))
                      {
                          return;
                      }

                      if (requireSession(*this, sessionId, response) == nullptr)
                      {
                          return;
                      }

                      const std::string investigationId = request.matches[1];
                      const auto manifestResult = m_workspaceStore.investigationStore().get(investigationId);

                      if (!manifestResult)
                      {
                          setErrorResponse(response, manifestResult.error());

                          return;
                      }

                      setJsonResponse(response, 200, successEnvelope(formatInvestigationManifest(*manifestResult)));
                      response.set_header(kSessionHeader, sessionId);
                  });

    m_server->Get(R"(/api/v1/investigations/([^/]+)/timeline)",
                  [this](const httplib::Request& request, httplib::Response& response) {
                      if (!authorizeApiKey(m_config.apiKey, request, response))
                      {
                          return;
                      }

                      applyCors(m_config, request, response);

                      const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                      if (rejectStaleSessionHeader(request, sessionId, response))
                      {
                          return;
                      }

                      if (requireSession(*this, sessionId, response) == nullptr)
                      {
                          return;
                      }

                      const std::string investigationId = request.matches[1];
                      const std::string limitValue = request.has_param("limit") ? request.get_param_value("limit") : "";
                      const std::string offsetValue =
                          request.has_param("offset") ? request.get_param_value("offset") : "";
                      const std::string orderValue = request.has_param("order") ? request.get_param_value("order") : "";
                      const InvestigationTimelineQuery timelineQuery =
                          parseInvestigationTimelineQuery(limitValue, offsetValue, orderValue);
                      const auto timelineResult = m_workspaceStore.investigationStore().projectTimeline(
                          investigationId, timelineQuery.options);

                      if (!timelineResult)
                      {
                          setErrorResponse(response, timelineResult.error());

                          return;
                      }

                      setJsonResponse(response, 200,
                                      successEnvelope(formatInvestigationTimeline(investigationId, *timelineResult)));
                      response.set_header(kSessionHeader, sessionId);
                  });

    m_server->Get(R"(/api/v1/investigations/([^/]+)/artifacts/([^/]+)/crash-analysis)",
                  [this](const httplib::Request& request, httplib::Response& response) {
                      if (!authorizeApiKey(m_config.apiKey, request, response))
                      {
                          return;
                      }

                      applyCors(m_config, request, response);

                      const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                      if (rejectStaleSessionHeader(request, sessionId, response))
                      {
                          return;
                      }

                      if (requireSession(*this, sessionId, response) == nullptr)
                      {
                          return;
                      }

                      const std::string investigationId = request.matches[1];
                      const std::string artifactId = request.matches[2];
                      const auto crashResult =
                          m_workspaceStore.investigationStore().analyzeCrash(investigationId, artifactId);

                      if (!crashResult)
                      {
                          if (crashResult.error().code() == foundation::ErrorCode::InvalidArgument &&
                              crashResult.error().message() == "ARTIFACT_NOT_ANALYZABLE")
                          {
                              setJsonResponse(response, 409,
                                              errorEnvelope("ARTIFACT_NOT_ANALYZABLE",
                                                            "Artifact type does not support crash analysis."));

                              return;
                          }

                          setErrorResponse(response, crashResult.error());

                          return;
                      }

                      setJsonResponse(response, 200,
                                      successEnvelope(formatInvestigationCrashAnalysis(*crashResult)));
                      response.set_header(kSessionHeader, sessionId);
                  });

    m_server->Put(R"(/api/v1/investigations/([^/]+))",
                [this](const httplib::Request& request, httplib::Response& response) {
                    if (!authorizeApiKey(m_config.apiKey, request, response))
                    {
                        return;
                    }

                    applyCors(m_config, request, response);

                    const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                    if (rejectStaleSessionHeader(request, sessionId, response))
                    {
                        return;
                    }

                    if (requireSession(*this, sessionId, response) == nullptr)
                    {
                        return;
                    }

                    const std::string investigationId = request.matches[1];
                    const InvestigationUpdateRequest updateRequest = parseInvestigationUpdateRequest(request.body);
                    const auto updateResult =
                        m_workspaceStore.investigationStore().update(investigationId, updateRequest);

                    if (!updateResult)
                    {
                        setErrorResponse(response, updateResult.error());

                        return;
                    }

                    setJsonResponse(response, 200, successEnvelope(formatInvestigationManifest(*updateResult)));
                    response.set_header(kSessionHeader, sessionId);
                });

    m_server->Delete(R"(/api/v1/investigations/([^/]+))",
                     [this](const httplib::Request& request, httplib::Response& response) {
                         if (!authorizeApiKey(m_config.apiKey, request, response))
                         {
                             return;
                         }

                         applyCors(m_config, request, response);

                         const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                         if (rejectStaleSessionHeader(request, sessionId, response))
                         {
                             return;
                         }

                         if (requireSession(*this, sessionId, response) == nullptr)
                         {
                             return;
                         }

                         const std::string investigationId = request.matches[1];
                         const auto removeResult = m_workspaceStore.investigationStore().remove(investigationId);

                         if (!removeResult)
                         {
                             setErrorResponse(response, removeResult.error());

                             return;
                         }

                         setJsonResponse(response, 200, successEnvelope("{\"deleted\": true}"));
                         response.set_header(kSessionHeader, sessionId);
                     });

    m_server->Post(R"(/api/v1/investigations/([^/]+)/artifacts)",
                   [this](const httplib::Request& request, httplib::Response& response) {
                       if (!authorizeApiKey(m_config.apiKey, request, response))
                       {
                           return;
                       }

                       applyCors(m_config, request, response);

                       const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                       if (rejectStaleSessionHeader(request, sessionId, response))
                       {
                           return;
                       }

                       WorkspaceSession* workspace = requireSession(*this, sessionId, response);

                       if (workspace == nullptr)
                       {
                           return;
                       }

                       const std::string investigationId = request.matches[1];
                       const auto artifactRequest = parseArtifactAddRequest(request.body);

                       if (!artifactRequest)
                       {
                           setErrorResponse(response, artifactRequest.error());

                           return;
                       }

                       if (artifactRequest->type == "note")
                       {
                           const auto noteResult = m_workspaceStore.investigationStore().addNoteArtifact(
                               investigationId, artifactRequest->name, artifactRequest->body);

                           if (!noteResult)
                           {
                               setErrorResponse(response, noteResult.error());

                               return;
                           }

                           setJsonResponse(response, 200, successEnvelope(formatArtifactRecord(*noteResult)));
                           response.set_header(kSessionHeader, sessionId);

                           return;
                       }

                       foundation::Path sourcePath = foundation::Path(artifactRequest->sourcePath);
                       const bool useSessionSource =
                           sourcePath.string().empty() && artifactRequest->type == "log";

                       if (useSessionSource)
                       {
                           std::lock_guard<std::mutex> lock(workspace->mutex);
                           sourcePath = workspace->service->sourcePath();
                       }

                       if (sourcePath.string().empty())
                       {
                           setJsonResponse(response, 400,
                                           errorEnvelope("INVALID_ARGUMENT", "Missing artifact source path."));

                           return;
                       }

                       if (!useSessionSource)
                       {
                           const auto pathValidation = validateServerPath(m_config, sourcePath);

                           if (!pathValidation)
                           {
                               setErrorResponse(response, pathValidation.error());

                               return;
                           }
                       }

                       const std::string displayName = artifactRequest->displayName.empty()
                                                           ? artifactRequest->name
                                                           : artifactRequest->displayName;

                       const auto fileResult = m_workspaceStore.investigationStore().addArtifactFile(
                           investigationId, sourcePath, displayName, artifactRequest->type, artifactRequest->role);

                       if (!fileResult)
                       {
                           setErrorResponse(response, fileResult.error());

                           return;
                       }

                       setJsonResponse(response, 200, successEnvelope(formatArtifactRecord(*fileResult)));
                       response.set_header(kSessionHeader, sessionId);
                   });

    m_server->Get(R"(/api/v1/investigations/([^/]+)/artifacts/([^/]+))",
                  [this](const httplib::Request& request, httplib::Response& response) {
                      if (!authorizeApiKey(m_config.apiKey, request, response))
                      {
                          return;
                      }

                      applyCors(m_config, request, response);

                      const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                      if (rejectStaleSessionHeader(request, sessionId, response))
                      {
                          return;
                      }

                      if (requireSession(*this, sessionId, response) == nullptr)
                      {
                          return;
                      }

                      const std::string investigationId = request.matches[1];
                      const std::string artifactId = request.matches[2];
                      const auto manifestResult = m_workspaceStore.investigationStore().get(investigationId);

                      if (!manifestResult)
                      {
                          setErrorResponse(response, manifestResult.error());

                          return;
                      }

                      const auto iterator = std::find_if(manifestResult->artifacts.begin(),
                                                         manifestResult->artifacts.end(),
                                                         [&artifactId](const scope::workspace::ArtifactRecord& artifact) {
                                                             return artifact.id == artifactId;
                                                         });

                      if (iterator == manifestResult->artifacts.end())
                      {
                          setJsonResponse(response, 404, errorEnvelope("NOT_FOUND", "Artifact not found."));

                          return;
                      }

                      std::ostringstream data;
                      data << "{\n  \"artifact\": "
                           << formatArtifactRecord(*iterator, manifestResult->primaryArtifactId);

                      if (iterator->type == "note")
                      {
                          const foundation::Path investigationDir = foundation::Path(
                              m_workspaceStore.investigationStore().rootDirectory().string() + "/" + investigationId);
                          const auto investigationResult = scope::workspace::Investigation::open(investigationDir);

                          if (investigationResult)
                          {
                              const scope::workspace::IArtifactHandler* handler =
                                  scope::workspace::findArtifactHandler("note");

                              if (handler != nullptr)
                              {
                                  const auto dataPathResult =
                                      handler->resolveDataPath(investigationResult->rootDirectory(), *iterator);

                                  if (dataPathResult)
                                  {
                                      std::ifstream stream(dataPathResult->string());

                                      if (stream)
                                      {
                                          std::ostringstream bodyBuffer;
                                          bodyBuffer << stream.rdbuf();
                                          data << ",\n  \"body\": \"" << escapeJsonString(bodyBuffer.str()) << '"';
                                      }
                                  }
                              }
                          }
                      }

                      data << "\n}";
                      setJsonResponse(response, 200, successEnvelope(data.str()));
                      response.set_header(kSessionHeader, sessionId);
                  });

    m_server->Delete(R"(/api/v1/investigations/([^/]+)/artifacts/([^/]+))",
                     [this](const httplib::Request& request, httplib::Response& response) {
                         if (!authorizeApiKey(m_config.apiKey, request, response))
                         {
                             return;
                         }

                         applyCors(m_config, request, response);

                         const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                         if (rejectStaleSessionHeader(request, sessionId, response))
                         {
                             return;
                         }

                         if (requireSession(*this, sessionId, response) == nullptr)
                         {
                             return;
                         }

                         const std::string investigationId = request.matches[1];
                         const std::string artifactId = request.matches[2];
                         const auto removeResult =
                             m_workspaceStore.investigationStore().removeArtifact(investigationId, artifactId);

                         if (!removeResult)
                         {
                             setErrorResponse(response, removeResult.error());

                             return;
                         }

                         setJsonResponse(response, 200, successEnvelope("{\"deleted\": true}"));
                         response.set_header(kSessionHeader, sessionId);
                     });

    m_server->Post(R"(/api/v1/investigations/([^/]+)/open)",
                   [this](const httplib::Request& request, httplib::Response& response) {
                       if (!authorizeApiKey(m_config.apiKey, request, response))
                       {
                           return;
                       }

                       applyCors(m_config, request, response);

                       const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

                       if (rejectStaleSessionHeader(request, sessionId, response))
                       {
                           return;
                       }

                       WorkspaceSession* workspace = requireSession(*this, sessionId, response);

                       if (workspace == nullptr)
                       {
                           return;
                       }

                       const std::string investigationId = request.matches[1];
                       const InvestigationOpenRequest openRequest = parseInvestigationOpenRequest(request.body);
                       const auto manifestResult = m_workspaceStore.investigationStore().get(investigationId);

                       if (!manifestResult)
                       {
                           setErrorResponse(response, manifestResult.error());

                           return;
                       }

                       std::string targetArtifactId = openRequest.artifactId;

                       if (targetArtifactId.empty())
                       {
                           targetArtifactId = manifestResult->primaryArtifactId;
                       }

                       const scope::workspace::ArtifactRecord* targetArtifact = nullptr;

                       for (const scope::workspace::ArtifactRecord& artifact : manifestResult->artifacts)
                       {
                           if (artifact.id == targetArtifactId)
                           {
                               targetArtifact = &artifact;
                               break;
                           }
                       }

                       if (targetArtifact == nullptr || targetArtifactId.empty())
                       {
                           setJsonResponse(response, 404,
                                           errorEnvelope("NOT_FOUND", "Artifact not found in investigation."));

                           return;
                       }

                       if (!scope::workspace::artifactTypeSupportsSessionOpen(targetArtifact->type))
                       {
                           setJsonResponse(response, 409,
                                           errorEnvelope("ARTIFACT_NOT_OPENABLE",
                                                           "Only log artifacts can be opened into the session."));

                           return;
                       }

                       const bool allowSnapshot =
                           openRequest.artifactId.empty() ||
                           openRequest.artifactId == manifestResult->primaryArtifactId;
                       const auto snapshotPath =
                           m_workspaceStore.investigationStore().resolveSnapshotPath(investigationId);
                       const auto targetLogPath = m_workspaceStore.investigationStore().resolveLogArtifactPath(
                           investigationId, targetArtifactId);

                       std::lock_guard<std::mutex> lock(workspace->mutex);

                       bool loadedFromSnapshot = false;

                       if (allowSnapshot && snapshotPath)
                       {
                           std::error_code errorCode;

                           if (std::filesystem::exists(snapshotPath->string(), errorCode))
                           {
                               const auto loadResult = workspace->service->loadSession(*snapshotPath);

                               if (loadResult)
                               {
                                   removeTempUploadFile(*workspace);
                                   workspace->service->adoptModel(loadResult->analysisModel(),
                                                                  loadResult->sourcePath());

                                   if (!loadResult->configFile().string().empty())
                                   {
                                       const auto configLoadResult =
                                           workspace->service->loadConfiguration(loadResult->configFile());

                                       if (!configLoadResult)
                                       {
                                           setErrorResponse(response, configLoadResult.error());

                                           return;
                                       }
                                   }

                                   loadedFromSnapshot = true;
                               }
                           }
                       }

                       if (!loadedFromSnapshot)
                       {
                           if (!targetLogPath)
                           {
                               setErrorResponse(response, targetLogPath.error());

                               return;
                           }

                           const auto openResult = workspace->service->openSource(*targetLogPath);

                           if (!openResult)
                           {
                               setErrorResponse(response, openResult.error());

                               return;
                           }

                           removeTempUploadFile(*workspace);
                       }

                       workspace->boundInvestigationId = investigationId;
                       workspace->activeArtifactId = targetArtifactId;

                       m_workspaceStore.investigationStore().touchUpdatedAt(investigationId);

                       scope::workspace::InvestigationSummary summary;

                       if (workspace->service->hasModel())
                       {
                           summary.hasModel = true;
                           summary.lineCount = workspace->service->model().totalLines();
                           summary.errorCount = workspace->service->model().levelCounts().errorLines();
                       }

                       setJsonResponse(response, 200,
                                       successEnvelope(formatInvestigationOpenResult(
                                           investigationId, targetArtifactId, targetArtifact->type,
                                           workspace->service->sourcePath(), summary, loadedFromSnapshot)));
                       response.set_header(kSessionHeader, sessionId);
                   });

    m_server->Post("/api/v1/tail/start", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);

        if (workspace->service->isTailing())
        {
            setJsonResponse(response, 409, errorEnvelope("INVALID_STATE", "Tail is already active."));

            return;
        }

        const auto startResult = workspace->service->startTail();

        if (!startResult)
        {
            setErrorResponse(response, startResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope("{\"active\": true}"));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/tail/stop", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);
        workspace->service->stopTail();
        setJsonResponse(response, 200, successEnvelope("{\"active\": false}"));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Get("/api/v1/tail/poll", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);

        if (!workspace->service->isTailing())
        {
            setJsonResponse(response, 409, errorEnvelope("INVALID_STATE", "Tail is not active."));

            return;
        }

        const auto pollResult = workspace->service->pollTailLines();

        if (!pollResult)
        {
            setErrorResponse(response, pollResult.error());

            return;
        }

        setJsonResponse(response, 200,
                        successEnvelope(formatTailPollResult(*pollResult, workspace->service->isTailing())));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Get(R"(/api/v1/jobs/([^/]+))", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        if (requireSession(*this, sessionId, response) == nullptr)
        {
            return;
        }

        const std::string jobId = request.matches[1];
        m_jobQueue.evictExpired();
        const auto pollResult = m_jobQueue.poll(sessionId, jobId);

        if (!pollResult)
        {
            setErrorResponse(response, pollResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(*pollResult));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Get("/api/v1/extensions", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(workspace->mutex);
        const std::vector<extension::ExtensionInfo> extensions = workspace->service->listExtensions();
        setJsonResponse(response, 200, successEnvelope(formatExtensionList(extensions)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Get(R"(/api/v1/extensions/([^/]+))", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        const std::string extensionId = request.matches[1];

        std::lock_guard<std::mutex> lock(workspace->mutex);
        const auto describeResult = workspace->service->describeExtension(extensionId);

        if (!describeResult)
        {
            setErrorResponse(response, describeResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(formatExtensionInfo(*describeResult)));
        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/export", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        const reporting::ReportOptions options = parseReportOptions(request.body);

        std::lock_guard<std::mutex> lock(workspace->mutex);

        if (!workspace->service->hasModel())
        {
            setJsonResponse(response, 409, errorEnvelope("INVALID_STATE", "Analyze a source before export."));

            return;
        }

        const reporting::Report report = workspace->service->generateReport(options);

        if (report.isBinary())
        {
            response.status = 200;
            response.set_content(reinterpret_cast<const char*>(report.bytes().data()), report.bytes().size(),
                               report.mimeType().c_str());
        }
        else if (options.format == reporting::ReportFormat::Html)
        {
            response.status = 200;
            response.set_content(report.text(), "text/html; charset=utf-8");
        }
        else if (options.format == reporting::ReportFormat::Json)
        {
            setJsonResponse(response, 200, successEnvelope(report.text()));
        }
        else
        {
            response.status = 200;
            response.set_content(report.text(), "text/plain; charset=utf-8");
        }

        response.set_header(kSessionHeader, sessionId);
    });

    m_server->Post("/api/v1/agent/investigate", [this](const httplib::Request& request, httplib::Response& response) {
        if (!authorizeApiKey(m_config.apiKey, request, response))
        {
            return;
        }

        applyCors(m_config, request, response);

        const std::string sessionId = resolveSessionId(m_sessionStore, request, true);

        if (rejectStaleSessionHeader(request, sessionId, response))
        {
            return;
        }

        WorkspaceSession* workspace = requireSession(*this, sessionId, response);

        if (workspace == nullptr)
        {
            return;
        }

        const AgentInvestigateRequest agentRequest = parseAgentInvestigateRequest(request.body);

        std::lock_guard<std::mutex> lock(workspace->mutex);
        const auto agentResult = workspace->service->agentInvestigate(agentRequest.criteria, agentRequest.askQuery,
                                                                      agentRequest.summarize, agentRequest.hints);

        if (!agentResult)
        {
            setErrorResponse(response, agentResult.error());

            return;
        }

        setJsonResponse(response, 200, successEnvelope(formatAgentInvestigateJson(*agentResult)));
        response.set_header(kSessionHeader, sessionId);
    });

    const std::filesystem::path uiDirectory = resolveUiDirectory();

    if (std::filesystem::is_directory(uiDirectory))
    {
        const auto serveUiFile = [this, uiDirectory](const std::string& relativePath, const char* contentType,
                                                     httplib::Response& response) {
            std::ifstream stream(uiDirectory / relativePath, std::ios::binary);

            if (!stream)
            {
                response.status = 404;

                return;
            }

            std::ostringstream buffer;
            buffer << stream.rdbuf();
            response.status = 200;
            response.set_content(buffer.str(), contentType);
        };

        m_server->Get("/", [this, serveUiFile](const httplib::Request& request, httplib::Response& response) {
            applyCors(m_config, request, response);
            serveUiFile("index.html", "text/html; charset=utf-8", response);
        });

        m_server->Get("/index.html", [this, serveUiFile](const httplib::Request& request, httplib::Response& response) {
            applyCors(m_config, request, response);
            serveUiFile("index.html", "text/html; charset=utf-8", response);
        });

        m_server->Get("/app.js", [this, serveUiFile](const httplib::Request& request, httplib::Response& response) {
            applyCors(m_config, request, response);
            serveUiFile("app.js", "application/javascript; charset=utf-8", response);
        });

        m_server->Get("/styles.css", [this, serveUiFile](const httplib::Request& request, httplib::Response& response) {
            applyCors(m_config, request, response);
            serveUiFile("styles.css", "text/css; charset=utf-8", response);
        });

        m_server->Get(R"(/.*)", [this, serveUiFile](const httplib::Request& request, httplib::Response& response) {
            if (request.path.rfind("/api/", 0) == 0)
            {
                response.status = 404;

                return;
            }

            applyCors(m_config, request, response);
            serveUiFile("index.html", "text/html; charset=utf-8", response);
        });
    }
}

} // namespace scope::web
