/**
 * @file evidence_link.cpp
 * @brief Evidence link type helpers.
 */

#include "evidence_link.hpp"

namespace scope::workspace
{

std::string evidenceLinkTypeToString(const EvidenceLinkType type)
{
    switch (type)
    {
    case EvidenceLinkType::Precedes:
        return "PRECEDES";
    case EvidenceLinkType::Follows:
        return "FOLLOWS";
    case EvidenceLinkType::Supports:
        return "SUPPORTS";
    case EvidenceLinkType::Related:
        return "RELATED";
    }

    return "RELATED";
}

std::optional<EvidenceLinkType> parseEvidenceLinkType(const std::string& value)
{
    if (value == "PRECEDES")
    {
        return EvidenceLinkType::Precedes;
    }

    if (value == "FOLLOWS")
    {
        return EvidenceLinkType::Follows;
    }

    if (value == "SUPPORTS")
    {
        return EvidenceLinkType::Supports;
    }

    if (value == "RELATED")
    {
        return EvidenceLinkType::Related;
    }

    return std::nullopt;
}

std::string evidenceLinkStatusToString(const EvidenceLinkStatus status)
{
    switch (status)
    {
    case EvidenceLinkStatus::Active:
        return "active";
    case EvidenceLinkStatus::Stale:
        return "stale";
    }

    return "active";
}

} // namespace scope::workspace
