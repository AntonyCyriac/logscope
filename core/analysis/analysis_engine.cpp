/**
 * @file analysis_engine.cpp
 * @brief AnalysisEngine implementation.
 */

#include "analysis_engine.hpp"

#include <string>

#include "foundation/error.hpp"
#include "log_line_classifier.hpp"
#include "log_macros.hpp"
#include "log_source.hpp"

namespace scope::analysis
{

namespace
{

void recordLevel(LogLevelCounts& counts, const DetectedLogLevel level) noexcept
{
    switch (level)
    {
    case DetectedLogLevel::Blank:
        counts.recordBlank();
        break;
    case DetectedLogLevel::Info:
        counts.recordInfo();
        break;
    case DetectedLogLevel::Warn:
        counts.recordWarn();
        break;
    case DetectedLogLevel::Error:
        counts.recordError();
        break;
    case DetectedLogLevel::Other:
        counts.recordOther();
        break;
    }
}

} // namespace

foundation::Result<AnalysisModel> AnalysisEngine::analyze(source::SourceDataset& dataset) const
{
    if (!dataset.isValid())
    {
        return foundation::Result<AnalysisModel>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Source dataset is not valid."));
    }

    SCOPE_LOG_INFO("analysis", "Analyzing source: " + dataset.path().string());

    std::string line;
    std::uint64_t totalLines = 0;
    LogLevelCounts levelCounts;
    source::LogSource& logSource = dataset.source();

    while (true)
    {
        const auto readResult = logSource.readLine(line);

        if (!readResult)
        {
            SCOPE_LOG_ERROR("analysis", readResult.error().message());

            return foundation::Result<AnalysisModel>(readResult.error());
        }

        if (!*readResult)
        {
            break;
        }

        ++totalLines;
        recordLevel(levelCounts, detectLogLevel(line));
    }

    SCOPE_LOG_INFO("analysis", "Counted " + std::to_string(totalLines) + " log lines");

    return foundation::Result<AnalysisModel>(AnalysisModel(dataset.path(), totalLines, levelCounts));
}

} // namespace scope::analysis
