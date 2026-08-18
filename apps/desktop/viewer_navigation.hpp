/**
 * @file viewer_navigation.hpp
 * @brief Cross-tab navigation contract for desktop investigation mode (P2).
 */

#pragma once

#include <cstddef>
#include <optional>
#include <QString>

namespace scope::desktop
{

/**
 * @brief Presentation-only navigation request — not a domain type.
 */
struct ViewerNavigation
{
    QString artifactId;
    std::optional<std::size_t> lineNumber;
    std::optional<QString> faultThreadId;
    std::optional<QString> peerEventId;
    QString targetTab;
    QString statusMessage;
};

} // namespace scope::desktop
