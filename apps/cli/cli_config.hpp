/**
 * @file cli_config.hpp
 * @brief CLI configuration initialization helpers.
 */

#pragma once

#include <iostream>

#include "configuration_manager.hpp"
#include "foundation/path.hpp"

namespace scope::cli
{

/**
 * @brief Loads configuration and applies diagnostics settings.
 */
[[nodiscard]] bool initializeConfiguration(const foundation::Path& configFilePath,
                                           configuration::ConfigurationManager& manager,
                                           std::ostream& errorOutput);

} // namespace scope::cli
