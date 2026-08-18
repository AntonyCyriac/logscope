/**
 * @file pstack_text_utils.hpp
 * @brief Pstack thread splitting for Crash tab viewer (P2).
 */

#pragma once

#include <QString>
#include <vector>

namespace scope::desktop
{

struct PstackThreadBlock
{
    QString id;
    QString text;
};

[[nodiscard]] std::vector<PstackThreadBlock> splitPstackThreads(const QString& text);

} // namespace scope::desktop
