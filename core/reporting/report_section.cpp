/**
 * @file report_section.cpp
 * @brief ReportSections implementation.
 */

#include "report_section.hpp"

#include <vector>

#include "foundation/string.hpp"

namespace scope::reporting
{

namespace
{

constexpr unsigned summaryBit = 1U << 0U;
constexpr unsigned levelBreakdownBit = 1U << 1U;
constexpr unsigned sourceMetadataBit = 1U << 2U;
constexpr unsigned allBits = summaryBit | levelBreakdownBit | sourceMetadataBit;

unsigned sectionBit(const ReportSection section) noexcept
{
    switch (section)
    {
    case ReportSection::Summary:
        return summaryBit;
    case ReportSection::LevelBreakdown:
        return levelBreakdownBit;
    case ReportSection::SourceMetadata:
        return sourceMetadataBit;
    }

    return 0U;
}

bool parseSectionName(const std::string& name, ReportSections& sections)
{
    const std::string normalized = foundation::toLower(foundation::trim(name));

    if (normalized == "summary")
    {
        sections.enable(ReportSection::Summary);

        return true;
    }

    if (normalized == "levels" || normalized == "level-breakdown")
    {
        sections.enable(ReportSection::LevelBreakdown);

        return true;
    }

    if (normalized == "metadata" || normalized == "source-metadata")
    {
        sections.enable(ReportSection::SourceMetadata);

        return true;
    }

    if (normalized == "all")
    {
        sections = ReportSections::all();

        return true;
    }

    return false;
}

} // namespace

ReportSections ReportSections::all() noexcept
{
    return ReportSections(allBits);
}

std::optional<ReportSections> ReportSections::parse(const std::string_view value)
{
    if (foundation::isBlank(value))
    {
        return std::nullopt;
    }

    ReportSections sections;
    sections.m_mask = 0U;

    const std::vector<std::string> parts = foundation::split(value, ',');

    for (const std::string& part : parts)
    {
        if (!parseSectionName(part, sections))
        {
            return std::nullopt;
        }
    }

    if (sections.m_mask == 0U)
    {
        return std::nullopt;
    }

    return sections;
}

ReportSections::ReportSections(const unsigned mask) noexcept : m_mask(mask)
{
}

bool ReportSections::includes(const ReportSection section) const noexcept
{
    return (m_mask & sectionBit(section)) != 0U;
}

void ReportSections::enable(const ReportSection section) noexcept
{
    m_mask |= sectionBit(section);
}

void ReportSections::disable(const ReportSection section) noexcept
{
    m_mask &= ~sectionBit(section);
}

} // namespace scope::reporting
