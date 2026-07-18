/**
 * @file report.cpp
 * @brief Report implementation.
 */

#include "report.hpp"

namespace scope::reporting
{

Report::Report(std::string text) : m_text(std::move(text))
{
}

const std::string& Report::text() const noexcept
{
    return m_text;
}

} // namespace scope::reporting
