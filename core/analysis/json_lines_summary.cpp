/**
 * @file json_lines_summary.cpp
 * @brief JsonLinesSummary implementation.
 */

#include "json_lines_summary.hpp"

#include <algorithm>

namespace scope::analysis
{

void JsonLinesSummary::recordValidLine(const std::vector<std::string>& topLevelKeys)
{
    ++m_validLines;

    for (const std::string& key : topLevelKeys)
    {
        ++m_keyCounts[key];
    }
}

void JsonLinesSummary::recordParseFailure() noexcept
{
    ++m_parseFailures;
}

std::uint64_t JsonLinesSummary::validLines() const noexcept
{
    return m_validLines;
}

std::uint64_t JsonLinesSummary::parseFailures() const noexcept
{
    return m_parseFailures;
}

std::vector<std::pair<std::string, std::uint64_t>> JsonLinesSummary::topLevelKeys(const std::size_t limit) const
{
    std::vector<std::pair<std::string, std::uint64_t>> keys;
    keys.reserve(m_keyCounts.size());

    for (const auto& entry : m_keyCounts)
    {
        keys.emplace_back(entry.first, entry.second);
    }

    std::sort(keys.begin(),
              keys.end(),
              [](const std::pair<std::string, std::uint64_t>& left,
                 const std::pair<std::string, std::uint64_t>& right) {
                  if (left.second != right.second)
                  {
                      return left.second > right.second;
                  }

                  return left.first < right.first;
              });

    if (keys.size() > limit)
    {
        keys.resize(limit);
    }

    return keys;
}

} // namespace scope::analysis
