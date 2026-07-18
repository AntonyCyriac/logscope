/**
 * @file datetime.cpp
 * @brief DateTime implementation.
 */

#include "datetime.hpp"

#include "error.hpp"

namespace scope::foundation
{

Result<DateTime> DateTime::parse(std::string_view value)
{
    const std::size_t separator = value.find('T');

    if (separator == std::string_view::npos)
    {
        return Result<DateTime>(Error(ErrorCode::InvalidArgument, "Invalid date-time format."));
    }

    auto dateResult = Date::parse(value.substr(0, separator));

    if (!dateResult)
    {
        return Result<DateTime>(dateResult.error());
    }

    auto timeResult = Time::parse(value.substr(separator + 1));

    if (!timeResult)
    {
        return Result<DateTime>(timeResult.error());
    }

    return Result<DateTime>(DateTime(*dateResult, *timeResult));
}

std::string DateTime::toString() const
{
    return m_date.toString() + 'T' + m_time.toString();
}

} // namespace scope::foundation
