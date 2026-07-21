/**
 * @file field_filter.cpp
 * @brief FieldFilter implementation.
 */

#include "field_filter.hpp"

#include "foundation/string.hpp"

namespace scope::investigation
{

FieldFilter FieldFilter::any() noexcept
{
    return FieldFilter{};
}

FieldFilter FieldFilter::withLevel(const analysis::DetectedLogLevel level) const noexcept
{
    FieldFilter copy = *this;
    copy.m_level = level;

    return copy;
}

FieldFilter FieldFilter::withMessageContains(const std::string_view substring) const noexcept
{
    FieldFilter copy = *this;
    copy.m_messageContains = std::string(substring);

    return copy;
}

FieldFilter FieldFilter::withRequiredJsonKey(const std::string_view key) const noexcept
{
    FieldFilter copy = *this;
    copy.m_requiredJsonKey = std::string(key);

    return copy;
}

bool FieldFilter::isActive() const noexcept
{
    return m_level.has_value() || !m_messageContains.empty() || !m_requiredJsonKey.empty();
}

bool FieldFilter::matches(const analysis::IndexedLine& line) const noexcept
{
    if (!isActive())
    {
        return true;
    }

    if (m_level.has_value() && line.level != *m_level)
    {
        return false;
    }

    if (!m_messageContains.empty())
    {
        const std::string loweredMessage = foundation::toLower(line.messageExcerpt);
        const std::string loweredContent = foundation::toLower(line.contentExcerpt);
        const std::string loweredQuery = foundation::toLower(m_messageContains);

        if (loweredMessage.find(loweredQuery) == std::string::npos &&
            loweredContent.find(loweredQuery) == std::string::npos)
        {
            return false;
        }
    }

    if (!m_requiredJsonKey.empty())
    {
        bool foundKey = false;

        for (const std::string& key : line.topLevelKeys)
        {
            if (foundation::toLower(key) == foundation::toLower(m_requiredJsonKey))
            {
                foundKey = true;
                break;
            }
        }

        if (!foundKey)
        {
            return false;
        }
    }

    return true;
}

const std::optional<analysis::DetectedLogLevel>& FieldFilter::level() const noexcept
{
    return m_level;
}

const std::string& FieldFilter::messageContains() const noexcept
{
    return m_messageContains;
}

const std::string& FieldFilter::requiredJsonKey() const noexcept
{
    return m_requiredJsonKey;
}

} // namespace scope::investigation
