/**
 * @file field_summary.cpp
 * @brief FieldSummary implementation.
 */

#include "field_summary.hpp"

#include <algorithm>

#include "foundation/string.hpp"

namespace scope::analysis
{

void FieldSummary::recordTimestamp(const foundation::Timestamp& timestamp) noexcept
{
    ++m_linesWithTimestamp;

    if (!m_earliestTimestamp.has_value() || timestamp < *m_earliestTimestamp)
    {
        m_earliestTimestamp = timestamp;
    }

    if (!m_latestTimestamp.has_value() || *m_latestTimestamp < timestamp)
    {
        m_latestTimestamp = timestamp;
    }
}

void FieldSummary::recordMessage(const std::string_view message)
{
    if (foundation::isBlank(message))
    {
        return;
    }

    ++m_linesWithMessage;

    const std::string excerpt = normalizeMessageExcerpt(message);

    if (!excerpt.empty())
    {
        ++m_messageCounts[excerpt];
    }
}

const std::optional<foundation::Timestamp>& FieldSummary::earliestTimestamp() const noexcept
{
    return m_earliestTimestamp;
}

const std::optional<foundation::Timestamp>& FieldSummary::latestTimestamp() const noexcept
{
    return m_latestTimestamp;
}

std::uint64_t FieldSummary::linesWithTimestamp() const noexcept
{
    return m_linesWithTimestamp;
}

std::uint64_t FieldSummary::linesWithMessage() const noexcept
{
    return m_linesWithMessage;
}

std::vector<std::pair<std::string, std::uint64_t>> FieldSummary::topMessages(const std::size_t limit) const
{
    std::vector<std::pair<std::string, std::uint64_t>> messages;
    messages.reserve(m_messageCounts.size());

    for (const auto& entry : m_messageCounts)
    {
        messages.emplace_back(entry.first, entry.second);
    }

    std::sort(messages.begin(),
              messages.end(),
              [](const std::pair<std::string, std::uint64_t>& left,
                 const std::pair<std::string, std::uint64_t>& right) {
                  if (left.second != right.second)
                  {
                      return left.second > right.second;
                  }

                  return left.first < right.first;
              });

    if (messages.size() > limit)
    {
        messages.resize(limit);
    }

    return messages;
}

std::string normalizeMessageExcerpt(const std::string_view message)
{
    std::string_view trimmed = message;

    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.front())) != 0)
    {
        trimmed.remove_prefix(1U);
    }

    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back())) != 0)
    {
        trimmed.remove_suffix(1U);
    }

    if (trimmed.size() > maxMessageExcerptLength)
    {
        trimmed = trimmed.substr(0U, maxMessageExcerptLength);
    }

    return std::string(trimmed);
}

} // namespace scope::analysis
