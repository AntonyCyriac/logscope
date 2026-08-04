/**
 * @file web_config.hpp
 * @brief Runtime configuration for logscope-web (M15.1).
 */

#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

#include "configuration_manager.hpp"
#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "middleware/api_key_credential.hpp"

namespace scope::web
{

/**
 * @brief Resolved web.* settings for the HTTP server.
 */
struct WebConfig
{
    std::uint64_t maxUploadBytes = 256ULL * 1024ULL * 1024ULL;
    std::uint64_t asyncAnalyzeThresholdBytes = 10ULL * 1024ULL * 1024ULL;
    std::vector<std::string> corsOrigins{"http://127.0.0.1:8080", "http://localhost:8080"};
    std::vector<foundation::Path> allowedPathRoots;
    std::string bindHost = "127.0.0.1";
    ApiKeyCredential apiKey;
    foundation::Path uploadTempDir;
    foundation::Path workspaceDir;
    foundation::Path tlsCertPath;
    foundation::Path tlsKeyPath;
    int bindPort = 8080;
    int requestTimeoutSeconds = 300;
    int workspacesListLimit = 100;
    int jobTtlSeconds = 3600;
    int jobMaxConcurrentPerSession = 1;
    int sessionTtlSeconds = 0;
    int maxSessions = 0;
    bool allowServerPaths = false;
    bool healthRequiresApiKey = false;
    bool corsOriginsUserSet = false;
    bool workspaceDirUserSet = false;

    std::string configuredApiKey;
    std::string configuredApiKeyHash;
    bool envApiKeySet = false;
    std::string envApiKey;
    bool envApiKeyHashSet = false;
    std::string envApiKeyHash;

    [[nodiscard]] static WebConfig defaults();

    /**
     * @brief Applies web.* keys from a loaded configuration manager.
     */
    void mergeFromConfiguration(const configuration::ConfigurationManager& configurationManager);

    /**
     * @brief Applies LOGSCOPE_WEB_* environment overrides.
     */
    void applyEnvironment();

    /**
     * @brief Resolves {@code apiKey} from config file values and environment overrides.
     *
     * Call after {@code mergeFromConfiguration} and {@code applyEnvironment}.
     */
    [[nodiscard]] foundation::Result<bool> finalizeApiKey();

    /**
     * @brief When web.cors_origins was not set explicitly, build http/https origins from bind host/port.
     */
    void applyDerivedDefaults();

    [[nodiscard]] bool tlsEnabled() const;

    [[nodiscard]] const char* urlScheme() const noexcept;

    [[nodiscard]] std::string listenUrl() const;

    [[nodiscard]] static bool isLoopbackBindHost(const std::string& host);
};

} // namespace scope::web
