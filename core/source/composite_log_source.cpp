/**
 * @file composite_log_source.cpp
 * @brief CompositeLogSource implementation.
 */

#include "composite_log_source.hpp"

#include "foundation/error.hpp"
#include "log_macros.hpp"

namespace scope::source
{

CompositeLogSource::CompositeLogSource(foundation::Path displayPath,
                                       std::vector<std::unique_ptr<LogSource>> sources)
    : m_displayPath(std::move(displayPath)), m_sources(std::move(sources))
{
}

foundation::Result<std::unique_ptr<LogSource>> CompositeLogSource::create(
    foundation::Path displayPath, std::vector<std::unique_ptr<LogSource>> sources)
{
    if (sources.empty())
    {
        return foundation::Result<std::unique_ptr<LogSource>>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "No log sources to open."));
    }

    SCOPE_LOG_DEBUG("source.composite",
                    "Created composite source for " + displayPath.string() + " with " +
                        std::to_string(sources.size()) + " file(s)");

    return foundation::Result<std::unique_ptr<LogSource>>(std::unique_ptr<LogSource>(
        new CompositeLogSource(std::move(displayPath), std::move(sources))));
}

const foundation::Path& CompositeLogSource::path() const noexcept
{
    return m_displayPath;
}

foundation::Result<bool> CompositeLogSource::readLine(std::string& line)
{
    while (m_currentIndex < m_sources.size())
    {
        const auto readResult = m_sources[m_currentIndex]->readLine(line);

        if (!readResult)
        {
            return readResult;
        }

        if (*readResult)
        {
            return foundation::Result<bool>(true);
        }

        ++m_currentIndex;
    }

    return foundation::Result<bool>(false);
}

} // namespace scope::source
