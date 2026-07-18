/**
 * @file diagnostics.hpp
 * @brief Diagnostic logging.
 */

#pragma once

#include <iosfwd>
#include <string_view>

#include "foundation/result.hpp"

namespace scope::runtime
{

class Configuration;

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
 * @brief Returns the string label for a log level.
 *
 * @param level Log level.
 * @return Level label (DEBUG, INFO, WARN, or ERROR).
 */
[[nodiscard]] const char* toString(LogLevel level) noexcept;

/**
 * @brief Parses a log level from a string.
 *
 * Accepts debug, info, warn/warning, and error (case-insensitive).
 *
 * @param value Log level string.
 * @return Parsed level or error.
 */
[[nodiscard]] foundation::Result<LogLevel> parseLogLevel(std::string_view value);

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
     * @brief Returns the minimum log level.
     */
    [[nodiscard]] LogLevel minLevel() const noexcept;

    /**
     * @brief Sets the minimum log level.
     *
     * Messages below this level are suppressed.
     *
     * @param level Minimum log level.
     */
    void setMinLevel(LogLevel level) noexcept;

    /**
     * @brief Sets the output stream used for log messages.
     *
     * @param stream Output stream. nullptr restores the default (stderr).
     */
    void setOutputStream(std::ostream* stream) noexcept;

    /**
     * @brief Returns whether timestamps are included in log output.
     */
    [[nodiscard]] bool timestampsEnabled() const noexcept;

    /**
     * @brief Enables or disables UTC timestamps in log output.
     *
     * @param enabled true to include timestamps.
     */
    void setTimestampsEnabled(bool enabled) noexcept;

    /**
     * @brief Applies log settings from configuration.
     *
     * Reads the log.level and log.timestamps keys. Returns false if a present value is invalid.
     *
     * @param configuration Configuration store.
     * @return true if configuration was applied successfully.
     */
    bool applyConfiguration(const Configuration& configuration);

    /**
     * @brief Writes a diagnostic message.
     *
     * @param level Log level.
     * @param message Message text.
     */
    void log(LogLevel level, std::string_view message);

    /**
     * @brief Writes a category-tagged diagnostic message.
     *
     * @param level Log level.
     * @param category Category tag (e.g. cli, runtime.config).
     * @param message Message text.
     */
    void log(LogLevel level, std::string_view category, std::string_view message);

  private:
    Diagnostics() = default;

    [[nodiscard]] bool shouldLog(LogLevel level) const noexcept;

    void write(LogLevel level, std::string_view category, std::string_view message);

    LogLevel m_minLevel = LogLevel::Info;

    bool m_timestampsEnabled = true;

    std::ostream* m_outputStream = nullptr;
};

} // namespace scope::runtime
