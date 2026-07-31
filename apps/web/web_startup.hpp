/**
 * @file web_startup.hpp
 * @brief CLI and configuration loading for logscope-web startup.
 */

#pragma once

#include <ostream>
#include <string>

#include "foundation/result.hpp"
#include "web_config.hpp"

namespace scope::web
{

/**
 * @brief Builds final WebConfig from defaults, optional properties file, environment, and CLI.
 *
 * Precedence (last wins): defaults → --config file → environment → CLI flags → derived CORS defaults.
 */
[[nodiscard]] foundation::Result<WebConfig> loadStartupConfig(int argc, char* argv[]);

/**
 * @brief Prints usage for logscope-web.
 */
void printWebUsage(std::ostream& output);

} // namespace scope::web
