/**
 * @file date.cpp
 * @brief Date implementation.
 */

#include "date.hpp"

#include <iomanip>
#include <sstream>

#include "error.hpp"

namespace scope::foundation
{

namespace
{

bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

int parseDigits(std::string_view value, std::size_t index, int count)
{
    int result = 0;

    for (int i = 0; i < count; ++i)
    {
        result = result * 10 + (value[index + static_cast<std::size_t>(i)] - '0');
    }

    return result;
}

bool isLeapYear(int year)
{
    if (year % 400 == 0)
    {
        return true;
    }

    if (year % 100 == 0)
    {
        return false;
    }

    return year % 4 == 0;
}

int daysInMonth(int year, int month)
{
    static constexpr int DAYS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == 2 && isLeapYear(year))
    {
        return 29;
    }

    return DAYS[month - 1];
}

Result<Date> validateDate(int year, int month, int day)
{
    if (month < 1 || month > 12)
    {
        return Result<Date>(Error(ErrorCode::InvalidArgument, "Month out of range."));
    }

    if (day < 1 || day > daysInMonth(year, month))
    {
        return Result<Date>(Error(ErrorCode::InvalidArgument, "Day out of range."));
    }

    return Result<Date>(Date(year, month, day));
}

} // namespace

Result<Date> Date::parse(std::string_view value)
{
    if (value.size() != 10)
    {
        return Result<Date>(Error(ErrorCode::InvalidArgument, "Invalid date length."));
    }

    if (value[4] != '-' || value[7] != '-')
    {
        return Result<Date>(Error(ErrorCode::InvalidArgument, "Invalid date format."));
    }

    for (std::size_t i = 0; i < value.size(); ++i)
    {
        if (i == 4 || i == 7)
        {
            continue;
        }

        if (!isDigit(value[i]))
        {
            return Result<Date>(Error(ErrorCode::InvalidArgument, "Invalid date character."));
        }
    }

    const int year = parseDigits(value, 0, 4);
    const int month = parseDigits(value, 5, 2);
    const int day = parseDigits(value, 8, 2);

    return validateDate(year, month, day);
}

std::string Date::toString() const
{
    std::ostringstream stream;

    stream << std::setfill('0') << std::setw(4) << m_year << '-' << std::setw(2) << m_month
           << '-' << std::setw(2) << m_day;

    return stream.str();
}

} // namespace scope::foundation
