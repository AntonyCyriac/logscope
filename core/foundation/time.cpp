/**
 * @file time.cpp
 * @brief Time implementation.
 */

#include "time.hpp"

#include <iomanip>
#include <sstream>

#include "error.hpp"

namespace scope::foundation
{

namespace
{

constexpr int MAX_HOUR = 23;

constexpr int MAX_MINUTE = 59;

constexpr int MAX_SECOND = 59;

constexpr int MAX_NANOSECOND = 999999999;

bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

int parseTwoDigits(std::string_view value, std::size_t index)
{
    return (value[index] - '0') * 10 + (value[index + 1] - '0');
}

Result<Time> validateTime(int hour, int minute, int second, int nanosecond)
{
    if (hour < 0 || hour > MAX_HOUR)
    {
        return Result<Time>(Error(ErrorCode::InvalidArgument, "Hour out of range."));
    }

    if (minute < 0 || minute > MAX_MINUTE)
    {
        return Result<Time>(Error(ErrorCode::InvalidArgument, "Minute out of range."));
    }

    if (second < 0 || second > MAX_SECOND)
    {
        return Result<Time>(Error(ErrorCode::InvalidArgument, "Second out of range."));
    }

    if (nanosecond < 0 || nanosecond > MAX_NANOSECOND)
    {
        return Result<Time>(Error(ErrorCode::InvalidArgument, "Nanosecond out of range."));
    }

    return Result<Time>(Time(hour, minute, second, nanosecond));
}

} // namespace

Result<Time> Time::parse(std::string_view value)
{
    if (value.size() < 8)
    {
        return Result<Time>(Error(ErrorCode::InvalidArgument, "Invalid time length."));
    }

    if (value[2] != ':' || value[5] != ':')
    {
        return Result<Time>(Error(ErrorCode::InvalidArgument, "Invalid time format."));
    }

    if (!isDigit(value[0]) || !isDigit(value[1]) || !isDigit(value[3]) || !isDigit(value[4]) ||
        !isDigit(value[6]) || !isDigit(value[7]))
    {
        return Result<Time>(Error(ErrorCode::InvalidArgument, "Invalid time character."));
    }

    const int hour = parseTwoDigits(value, 0);
    const int minute = parseTwoDigits(value, 3);
    const int second = parseTwoDigits(value, 6);

    int nanosecond = 0;

    if (value.size() > 8)
    {
        if (value[8] != '.')
        {
            return Result<Time>(Error(ErrorCode::InvalidArgument, "Invalid time format."));
        }

        if (value.size() < 10)
        {
            return Result<Time>(Error(ErrorCode::InvalidArgument, "Invalid fractional seconds."));
        }

        std::string fraction(value.substr(9));

        while (fraction.size() < 9)
        {
            fraction.push_back('0');
        }

        if (fraction.size() > 9)
        {
            fraction = fraction.substr(0, 9);
        }

        for (char c : fraction)
        {
            if (!isDigit(c))
            {
                return Result<Time>(Error(ErrorCode::InvalidArgument, "Invalid time character."));
            }
        }

        nanosecond = std::stoi(fraction);
    }

    return validateTime(hour, minute, second, nanosecond);
}

std::string Time::toString() const
{
    std::ostringstream stream;

    stream << std::setfill('0') << std::setw(2) << m_hour << ':' << std::setw(2) << m_minute
           << ':' << std::setw(2) << m_second;

    return stream.str();
}

} // namespace scope::foundation
