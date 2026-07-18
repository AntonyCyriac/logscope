/**
 * @file duration.cpp
 * @brief Duration implementation.
 */

#include "duration.hpp"

#include <iomanip>
#include <climits>
#include <sstream>

#include "error.hpp"

namespace scope::foundation
{

namespace
{

constexpr int MAX_MINUTE = 59;

constexpr int MAX_SECOND = 59;

constexpr int MAX_NANOSECOND = 999999999;

constexpr std::int64_t NANOSECONDS_PER_SECOND = 1000000000LL;

bool isDigit(char character)
{
    return character >= '0' && character <= '9';
}

Result<int> parseNonNegativeInt(std::string_view value)
{
    if (value.empty())
    {
        return Result<int>(Error(ErrorCode::InvalidArgument, "Invalid duration component."));
    }

    for (char character : value)
    {
        if (!isDigit(character))
        {
            return Result<int>(Error(ErrorCode::InvalidArgument, "Invalid duration character."));
        }
    }

    if (value.size() > 1 && value.front() == '0')
    {
        return Result<int>(Error(ErrorCode::InvalidArgument, "Invalid duration component."));
    }

    std::int64_t parsed = 0;

    for (char character : value)
    {
        parsed = parsed * 10 + (character - '0');
    }

    if (parsed > static_cast<std::int64_t>(INT_MAX))
    {
        return Result<int>(Error(ErrorCode::InvalidArgument, "Duration component out of range."));
    }

    return Result<int>(static_cast<int>(parsed));
}

int parseTwoDigits(std::string_view value, std::size_t index)
{
    return (value[index] - '0') * 10 + (value[index + 1] - '0');
}

Result<Duration> validateComponents(int hours, int minutes, int seconds, int nanoseconds)
{
    if (hours < 0)
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Hours out of range."));
    }

    if (minutes < 0 || minutes > MAX_MINUTE)
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Minutes out of range."));
    }

    if (seconds < 0 || seconds > MAX_SECOND)
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Seconds out of range."));
    }

    if (nanoseconds < 0 || nanoseconds > MAX_NANOSECOND)
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Nanoseconds out of range."));
    }

    return Result<Duration>(Duration(hours, minutes, seconds, nanoseconds));
}

} // namespace

Result<Duration> Duration::fromNanoseconds(std::int64_t nanoseconds)
{
    if (nanoseconds < 0)
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Duration cannot be negative."));
    }

    return Result<Duration>(Duration(nanoseconds));
}

Result<Duration> Duration::fromSeconds(std::int64_t seconds)
{
    if (seconds < 0)
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Duration cannot be negative."));
    }

    return Result<Duration>(Duration(seconds * NANOSECONDS_PER_SECOND));
}

Result<Duration> Duration::parse(std::string_view value)
{
    const std::size_t firstColon = value.find(':');

    if (firstColon == std::string_view::npos)
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Invalid duration format."));
    }

    const std::size_t secondColon = value.find(':', firstColon + 1);

    if (secondColon == std::string_view::npos)
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Invalid duration format."));
    }

    if (secondColon + 2 >= value.size())
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Invalid duration length."));
    }

    const auto hoursResult = parseNonNegativeInt(value.substr(0, firstColon));

    if (!hoursResult)
    {
        return Result<Duration>(hoursResult.error());
    }

    const std::string_view minuteSecond = value.substr(firstColon + 1);

    if (minuteSecond.size() < 5 || minuteSecond[2] != ':')
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Invalid duration format."));
    }

    if (!isDigit(minuteSecond[0]) || !isDigit(minuteSecond[1]) || !isDigit(minuteSecond[3]) ||
        !isDigit(minuteSecond[4]))
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Invalid duration character."));
    }

    const int minutes = parseTwoDigits(minuteSecond, 0);
    const int seconds = parseTwoDigits(minuteSecond, 3);

    int nanoseconds = 0;

    if (minuteSecond.size() > 5)
    {
        if (minuteSecond[5] != '.')
        {
            return Result<Duration>(Error(ErrorCode::InvalidArgument, "Invalid duration format."));
        }

        if (minuteSecond.size() < 7)
        {
            return Result<Duration>(Error(ErrorCode::InvalidArgument, "Invalid fractional seconds."));
        }

        std::string fraction(minuteSecond.substr(6));

        while (fraction.size() < 9)
        {
            fraction.push_back('0');
        }

        if (fraction.size() > 9)
        {
            fraction = fraction.substr(0, 9);
        }

        for (char character : fraction)
        {
            if (!isDigit(character))
            {
                return Result<Duration>(Error(ErrorCode::InvalidArgument, "Invalid duration character."));
            }
        }

        nanoseconds = std::stoi(fraction);
    }

    return validateComponents(*hoursResult, minutes, seconds, nanoseconds);
}

std::string Duration::toString() const
{
    std::ostringstream stream;

    stream << hours() << ':' << std::setfill('0') << std::setw(2) << minutes() << ':' << std::setw(2) << seconds();

    return stream.str();
}

} // namespace scope::foundation
