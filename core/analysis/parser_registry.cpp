/**
 * @file parser_registry.cpp
 */

#include "parser_registry.hpp"

#include <algorithm>

namespace scope::analysis
{

ParserRegistry& ParserRegistry::instance()
{
    static ParserRegistry registry;

    return registry;
}

void ParserRegistry::registerParser(const std::string& formatId, FormatParserFactory factory)
{
    for (ParserEntry& entry : m_parsers)
    {
        if (entry.formatId == formatId)
        {
            entry.factory = std::move(factory);
            entry.cachedInstance.reset();

            return;
        }
    }

    ParserEntry entry;
    entry.formatId = formatId;
    entry.factory = std::move(factory);

    m_parsers.push_back(std::move(entry));
}

const FormatParser* ParserRegistry::findParser(const std::string& formatId) const
{
    for (ParserEntry& entry : m_parsers)
    {
        if (entry.formatId != formatId)
        {
            continue;
        }

        if (entry.cachedInstance == nullptr && entry.factory)
        {
            entry.cachedInstance = entry.factory();
        }

        return entry.cachedInstance.get();
    }

    return nullptr;
}

std::vector<std::string> ParserRegistry::registeredFormatIds() const
{
    std::vector<std::string> ids;
    ids.reserve(m_parsers.size());

    for (const ParserEntry& entry : m_parsers)
    {
        ids.push_back(entry.formatId);
    }

    return ids;
}

void ParserRegistry::clear()
{
    m_parsers.clear();
}

} // namespace scope::analysis
