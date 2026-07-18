/**
 * @file source_dataset.cpp
 * @brief SourceDataset implementation.
 */

#include "source_dataset.hpp"

#include "foundation/error.hpp"

namespace scope::source
{

SourceDataset::SourceDataset(std::unique_ptr<LogSource> source) : m_source(std::move(source))
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

} // namespace scope::source
