/**
 * @file timestamp.hpp
 * @brief Unix timestamp value type.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "datetime.hpp"
#include "duration.hpp"
#include "result.hpp"

namespace scope::foundation
{

/**
 * @brief Represents an absolute point in time as Unix nanoseconds since epoch (UTC).
 */
class Timestamp
{
  public:
    /**
     * @brief Constructs the Unix epoch (1970-01-01T00:00:00Z).
     */
    constexpr Timestamp() noexcept;

    /**
     * @brief Constructs a timestamp from Unix nanoseconds since epoch.
     *
     * @param unixNanoseconds Nanoseconds since 1970-01-01T00:00:00Z.
     */
    constexpr explicit Timestamp(std::int64_t unixNanoseconds) noexcept;

    /**
     * @brief Constructs a timestamp from Unix seconds since epoch.
     *
     * @param unixSeconds Seconds since 1970-01-01T00:00:00Z.
     * @return Timestamp value.
     */
    [[nodiscard]] static constexpr Timestamp fromUnixSeconds(std::int64_t unixSeconds) noexcept;

    /**
     * @brief Constructs a timestamp from a calendar date-time (UTC).
     *
     * @param dateTime Calendar date and time.
     * @return Timestamp or error if the value is out of range.
     */
    [[nodiscard]] static Result<Timestamp> fromDateTime(const DateTime& dateTime);

    /**
     * @brief Parses an ISO-like date-time string (YYYY-MM-DDTHH:MM:SS).
     *
     * @param value Date-time string.
     * @return Parsed timestamp or error.
     */
    [[nodiscard]] static Result<Timestamp> parse(std::string_view value);

    /**
     * @brief Returns Unix nanoseconds since epoch.
     */
    [[nodiscard]] constexpr std::int64_t unixNanoseconds() const noexcept;

    /**
     * @brief Returns Unix seconds since epoch (truncated toward zero).
     */
    [[nodiscard]] constexpr std::int64_t unixSeconds() const noexcept;

    /**
     * @brief Converts the timestamp to a calendar date-time (UTC).
     *
     * @return Date-time or error if the value is out of range.
     */
    [[nodiscard]] Result<DateTime> toDateTime() const;

    /**
     * @brief Returns canonical ISO-like date-time string.
     */
    [[nodiscard]] std::string toString() const;

    friend constexpr bool operator==(const Timestamp& lhs, const Timestamp& rhs) noexcept;

    friend constexpr bool operator!=(const Timestamp& lhs, const Timestamp& rhs) noexcept;

    friend constexpr bool operator<(const Timestamp& lhs, const Timestamp& rhs) noexcept;

    friend constexpr bool operator<=(const Timestamp& lhs, const Timestamp& rhs) noexcept;

    friend constexpr Timestamp operator+(const Timestamp& lhs, const Duration& rhs) noexcept;

    friend constexpr Timestamp operator-(const Timestamp& lhs, const Duration& rhs) noexcept;

    /**
     * @brief Returns the elapsed duration between two timestamps.
     *
     * @param later Later timestamp.
     * @param earlier Earlier timestamp.
     * @return Elapsed duration or error if later is before earlier.
     */
    [[nodiscard]] static Result<Duration> difference(const Timestamp& later, const Timestamp& earlier);

  private:
    std::int64_t m_unixNanoseconds{0};
};

} // namespace scope::foundation

#include "timestamp.inl"
