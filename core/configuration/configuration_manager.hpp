/**
 * @file configuration_manager.hpp
 * @brief Loads, validates, and provides configuration to the system.
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "runtime/configuration.hpp"

namespace scope::configuration
{

/**
 * @brief Loads configuration from files and environment variables.
 *
 * Configuration files use a simple key=value format with one entry per line.
 * Lines beginning with '#' are treated as comments. Environment variables
 * prefixed with SCOPE_ override file values (underscores map to dots).
 */
class ConfigurationManager
{
  public:
    /**
     * @brief Constructs an empty configuration manager.
     */
    ConfigurationManager() = default;

    /**
     * @brief Loads configuration from a properties file.
     *
     * @param path Path to the configuration file.
     * @return Loaded manager or error.
     */
    [[nodiscard]] static foundation::Result<ConfigurationManager> loadFromFile(const foundation::Path& path);

    /**
     * @brief Applies SCOPE_-prefixed environment variables as overrides.
     */
    void applyEnvironment();

    /**
     * @brief Validates that all required keys are present.
     *
     * @param requiredKeys Keys that must exist in the configuration.
     * @return Success or error listing missing keys.
     */
    [[nodiscard]] foundation::Result<bool> validate(const std::vector<std::string>& requiredKeys) const;

    /**
     * @brief Returns the underlying configuration store.
     *
     * @return Constant reference to the configuration.
     */
    [[nodiscard]] const runtime::Configuration& configuration() const noexcept;

    /**
     * @brief Returns the underlying configuration store.
     *
     * @return Reference to the configuration.
     */
    [[nodiscard]] runtime::Configuration& configuration() noexcept;

  private:
    runtime::Configuration m_configuration;
};

/**
 * @brief Converts a SCOPE_-prefixed environment variable name to a config key.
 *
 * @param environmentVariable Environment variable name (e.g. SCOPE_LOG_LEVEL).
 * @return Config key (e.g. log.level) or empty if the prefix does not match.
 */
[[nodiscard]] std::string environmentVariableToConfigKey(std::string_view environmentVariable) noexcept;

} // namespace scope::configuration
