/**
 * @file source_dataset.cpp
 * @brief SourceDataset implementation.
 */

#include "source_dataset.hpp"

namespace scope::source
{

SourceDataset::SourceDataset(std::unique_ptr<LogSource> source) : m_source(std::move(source))
{
}

SourceDataset::SourceDataset(std::unique_ptr<LogSource> source, DiscoveryCensus census)
    : m_source(std::move(source)), m_discoveryCensus(std::move(census))
{
}

bool SourceDataset::isValid() const noexcept
{
    return m_source != nullptr;
}

const foundation::Path& SourceDataset::path() const
{
    return m_source->path();
}

LogSource& SourceDataset::source()
{
    return *m_source;
}

const LogSource& SourceDataset::source() const
{
    return *m_source;
}

bool SourceDataset::hasDiscoveryCensus() const noexcept
{
    return m_discoveryCensus.has_value();
}

const DiscoveryCensus& SourceDataset::discoveryCensus() const noexcept
{
    return *m_discoveryCensus;
}

AnalysisAccounting& SourceDataset::analysisAccounting() noexcept
{
    return m_analysisAccounting;
}

const AnalysisAccounting& SourceDataset::analysisAccounting() const noexcept
{
    return m_analysisAccounting;
}

} // namespace scope::source
