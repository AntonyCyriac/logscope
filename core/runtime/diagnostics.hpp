/**
 * @file diagnostics.hpp
 * @brief Diagnostic logging.
 */

#pragma once

#include <string_view>

namespace scope::runtime
{

/**
 * @brief Diagnostic log levels.
 */
enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};

/**
 * @brief Provides diagnostic logging for platform services.
 */
class Diagnostics
{
  public:
    /**
     * @brief Returns the global diagnostics instance.
     */
    static Diagnostics& instance();

    /**
     * @brief Writes a diagnostic message.
     *
     * @param level Log level.
     * @param message Message text.
     */
    void log(LogLevel level, std::string_view message);

  private:
    Diagnostics() = default;
};

} // namespace scope::runtime
