/**
 * @file investigation_criteria.cpp
 * @brief InvestigationCriteria implementation.
 */

#include "investigation_criteria.hpp"

#include "foundation/string.hpp"

namespace scope::investigation
{

bool InvestigationCriteria::isActive() const noexcept
{
    return !foundation::isBlank(contentSearch) || timeRange.isActive() || field.isActive();
}

} // namespace scope::investigation
