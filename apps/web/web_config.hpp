/**
 * @file web_config.hpp
 * @brief Runtime configuration for logscope-web (M15.1).
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "configuration_manager.hpp"
#include "foundation/path.hpp"

namespace scope::web
{

/**
 * @brief Resolved web.* settings for the HTTP server.
 */
struct WebConfig
{
    std::string bindHost = "127.0.0.1";
    int bindPort = 8080;
    std::string apiKey;
    std::vector<std::string> corsOrigins{"http://127.0.0.1:8080", "http://localhost:8080"};
    std::uint64_t maxUploadBytes = 256ULL * 1024ULL * 1024ULL;
    foundation::Path uploadTempDir;
    bool allowServerPaths = false;
    std::vector<foundation::Path> allowedPathRoots;
    int requestTimeoutSeconds = 300;

    [[nodiscard]] static WebConfig defaults();

    /**
     * @brief Applies web.* keys from a loaded configuration manager.
     */
    void mergeFromConfiguration(const configuration::ConfigurationManager& configurationManager);

    /**
     * @brief Applies LOGSCOPE_WEB_* environment overrides.
     */
    void applyEnvironment();
};

} // namespace scope::web
