/**
 * @file timestamp.cpp
 * @brief Timestamp implementation.
 */

#include "timestamp.hpp"

#include "error.hpp"

namespace scope::foundation
{

namespace
{

constexpr std::int64_t NANOSECONDS_PER_DAY = 86400000000000LL;

constexpr std::int64_t NANOSECONDS_PER_SECOND = 1000000000LL;

constexpr std::int64_t DAYS_FROM_UNIX_EPOCH = 719468LL;

std::int64_t daysFromCivil(int year, int month, int day)
{
    const int adjustedYear = year - (month <= 2 ? 1 : 0);
    const int era = (adjustedYear >= 0 ? adjustedYear : adjustedYear - 399) / 400;
    const unsigned yearOfEra = static_cast<unsigned>(adjustedYear - era * 400);
    const unsigned dayOfYear = (153 * (static_cast<unsigned>(month) + (month > 2 ? static_cast<unsigned>(-3) : 9)) +
                                2) /
                                   5 +
                               static_cast<unsigned>(day) - 1;
    const unsigned dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;

    return static_cast<std::int64_t>(era) * 146097LL + static_cast<std::int64_t>(dayOfEra) -
           DAYS_FROM_UNIX_EPOCH;
}

void civilFromDays(std::int64_t daysSinceEpoch, int& year, int& month, int& day)
{
    std::int64_t days = daysSinceEpoch + DAYS_FROM_UNIX_EPOCH;
    const std::int64_t era = (days >= 0 ? days : days - 146096) / 146097;
    const unsigned dayOfEra = static_cast<unsigned>(days - era * 146097);
    const unsigned yearOfEra = (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
    const unsigned dayOfYear = dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
    const unsigned monthIndex = (5 * dayOfYear + 2) / 153;
    const unsigned dayOfMonth = dayOfYear - (153 * monthIndex + 2) / 5 + 1;

    month = static_cast<int>(monthIndex + (monthIndex < 10 ? 3 : -9));
    year = static_cast<int>(yearOfEra) + static_cast<int>(era) * 400 + (month <= 2 ? 1 : 0);
    day = static_cast<int>(dayOfMonth);
}

Result<std::int64_t> dateTimeToUnixNanoseconds(const DateTime& dateTime)
{
    const std::int64_t days = daysFromCivil(dateTime.date().year(), dateTime.date().month(),
                                            dateTime.date().day());
    const std::int64_t timeNanoseconds =
        ((static_cast<std::int64_t>(dateTime.time().hour()) * 3600LL +
          static_cast<std::int64_t>(dateTime.time().minute()) * 60LL +
          static_cast<std::int64_t>(dateTime.time().second())) *
             NANOSECONDS_PER_SECOND) +
        static_cast<std::int64_t>(dateTime.time().nanosecond());

    const std::int64_t totalNanoseconds = days * NANOSECONDS_PER_DAY + timeNanoseconds;

    return Result<std::int64_t>(totalNanoseconds);
}

} // namespace

Result<Timestamp> Timestamp::fromDateTime(const DateTime& dateTime)
{
    const auto nanosecondsResult = dateTimeToUnixNanoseconds(dateTime);

    if (!nanosecondsResult)
    {
        return Result<Timestamp>(nanosecondsResult.error());
    }

    return Result<Timestamp>(Timestamp(*nanosecondsResult));
}

Result<Timestamp> Timestamp::parse(std::string_view value)
{
    const auto dateTimeResult = DateTime::parse(value);

    if (!dateTimeResult)
    {
        return Result<Timestamp>(dateTimeResult.error());
    }

    return fromDateTime(*dateTimeResult);
}

Result<DateTime> Timestamp::toDateTime() const
{
    const std::int64_t days = m_unixNanoseconds / NANOSECONDS_PER_DAY;
    const std::int64_t timeNanoseconds = m_unixNanoseconds % NANOSECONDS_PER_DAY;

    if (timeNanoseconds < 0)
    {
        return Result<DateTime>(Error(ErrorCode::InvalidArgument, "Timestamp out of range."));
    }

    int year = 0;
    int month = 0;
    int day = 0;

    civilFromDays(days, year, month, day);

    const std::int64_t totalSeconds = timeNanoseconds / NANOSECONDS_PER_SECOND;
    const int nanosecond = static_cast<int>(timeNanoseconds % NANOSECONDS_PER_SECOND);
    const int second = static_cast<int>(totalSeconds % 60);
    const int minute = static_cast<int>((totalSeconds / 60) % 60);
    const int hour = static_cast<int>(totalSeconds / 3600);

    if (hour > 23)
    {
        return Result<DateTime>(Error(ErrorCode::InvalidArgument, "Timestamp out of range."));
    }

    return Result<DateTime>(DateTime(Date(year, month, day), Time(hour, minute, second, nanosecond)));
}

std::string Timestamp::toString() const
{
    const auto dateTimeResult = toDateTime();

    if (!dateTimeResult)
    {
        return "invalid-timestamp";
    }

    return dateTimeResult->toString();
}

Result<Duration> Timestamp::difference(const Timestamp& later, const Timestamp& earlier)
{
    const std::int64_t elapsedNanoseconds = later.unixNanoseconds() - earlier.unixNanoseconds();

    if (elapsedNanoseconds < 0)
    {
        return Result<Duration>(Error(ErrorCode::InvalidArgument, "Later timestamp must not be before earlier."));
    }

    return Duration::fromNanoseconds(elapsedNanoseconds);
}

} // namespace scope::foundation
