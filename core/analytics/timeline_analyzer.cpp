/**
 * @file timeline_analyzer.cpp
 * @brief TimelineAnalyzer implementation.
 */

#include "timeline_analyzer.hpp"

#include <algorithm>

#include "foundation/duration.hpp"
#include "log_line_classifier.hpp"

namespace scope::analytics
{

namespace
{

void incrementLevel(analysis::LogLevelCounts& counts, const analysis::DetectedLogLevel level) noexcept
{
    switch (level)
    {
    case analysis::DetectedLogLevel::Blank:
        counts.recordBlank();
        break;
    case analysis::DetectedLogLevel::Info:
        counts.recordInfo();
        break;
    case analysis::DetectedLogLevel::Warn:
        counts.recordWarn();
        break;
    case analysis::DetectedLogLevel::Error:
        counts.recordError();
        break;
    case analysis::DetectedLogLevel::Other:
        counts.recordOther();
        break;
    }
}

std::int64_t durationSeconds(const foundation::Duration& duration) noexcept
{
    return duration.totalNanoseconds() / 1000000000LL;
}

std::int64_t resolveBucketSeconds(const analysis::AnalysisModel& model, const AnalyticsConfig& config)
{
    if (config.bucketSeconds.has_value() && *config.bucketSeconds > 0)
    {
        return *config.bucketSeconds;
    }

    if (!model.fieldSummary().has_value())
    {
        return 60;
    }

    const analysis::FieldSummary& summary = *model.fieldSummary();

    if (!summary.earliestTimestamp().has_value() || !summary.latestTimestamp().has_value())
    {
        return 60;
    }

    const foundation::Timestamp& earliest = *summary.earliestTimestamp();
    const foundation::Timestamp& latest = *summary.latestTimestamp();

    if (latest <= earliest)
    {
        return 60;
    }

    const auto spanResult = foundation::Timestamp::difference(latest, earliest);

    if (!spanResult)
    {
        return 60;
    }

    const std::int64_t spanSeconds = std::max<std::int64_t>(durationSeconds(*spanResult), 1);
    const std::int64_t bucketSeconds = spanSeconds / static_cast<std::int64_t>(defaultTimelineBucketCount);

    return std::max<std::int64_t>(bucketSeconds, 1);
}

} // namespace

TimelineResult TimelineAnalyzer::analyze(const analysis::AnalysisModel& model,
                                         const AnalyticsConfig& config) const
{
    TimelineResult result;

    if (!model.lineIndex().has_value())
    {
        return result;
    }

    foundation::Timestamp earliest;
    foundation::Timestamp latest;
    bool hasRange = false;

    if (model.fieldSummary().has_value())
    {
        if (model.fieldSummary()->earliestTimestamp().has_value() &&
            model.fieldSummary()->latestTimestamp().has_value())
        {
            earliest = *model.fieldSummary()->earliestTimestamp();
            latest = *model.fieldSummary()->latestTimestamp();
            hasRange = true;
        }
    }

    if (!hasRange)
    {
        for (const analysis::IndexedLine& line : model.lineIndex()->lines())
        {
            if (!line.timestamp.has_value())
            {
                continue;
            }

            if (!hasRange)
            {
                earliest = *line.timestamp;
                latest = *line.timestamp;
                hasRange = true;

                continue;
            }

            if (*line.timestamp < earliest)
            {
                earliest = *line.timestamp;
            }

            if (latest < *line.timestamp)
            {
                latest = *line.timestamp;
            }
        }
    }

    if (!hasRange)
    {
        return result;
    }

    const std::int64_t bucketSeconds = resolveBucketSeconds(model, config);
    result.setBucketSeconds(bucketSeconds);

    const auto spanResult = foundation::Timestamp::difference(latest, earliest);

    if (!spanResult)
    {
        return result;
    }

    const std::int64_t spanSeconds = std::max<std::int64_t>(durationSeconds(*spanResult), 1);
    const std::size_t bucketCount =
        static_cast<std::size_t>((spanSeconds / bucketSeconds) + (spanSeconds % bucketSeconds != 0 ? 1 : 0));
    const std::size_t resolvedBucketCount = std::max<std::size_t>(bucketCount, 1U);

    std::vector<TimelineBucket> buckets(resolvedBucketCount);

    for (std::size_t index = 0U; index < resolvedBucketCount; ++index)
    {
        const auto offsetResult = foundation::Duration::fromSeconds(bucketSeconds * static_cast<std::int64_t>(index));
        const auto bucketDurationResult = foundation::Duration::fromSeconds(bucketSeconds);

        if (!offsetResult || !bucketDurationResult)
        {
            continue;
        }

        buckets[index].start = earliest + *offsetResult;
        buckets[index].end = buckets[index].start + *bucketDurationResult;
        buckets[index].label = buckets[index].start.toString();
    }

    for (const analysis::IndexedLine& line : model.lineIndex()->lines())
    {
        if (!line.timestamp.has_value())
        {
            continue;
        }

        const auto offsetResult = foundation::Timestamp::difference(*line.timestamp, earliest);

        if (!offsetResult)
        {
            continue;
        }

        const std::int64_t offsetSeconds = durationSeconds(*offsetResult);
        const std::size_t bucketIndex = static_cast<std::size_t>(
            std::min<std::int64_t>(offsetSeconds / bucketSeconds, static_cast<std::int64_t>(resolvedBucketCount - 1)));

        incrementLevel(buckets[bucketIndex].levelCounts, line.level);
        ++buckets[bucketIndex].totalLines;
    }

    result.setBuckets(std::move(buckets));

    return result;
}

} // namespace scope::analytics
