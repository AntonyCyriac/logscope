/**
 * @file report_fragment.cpp
 * @brief ReportFragment implementation.
 */

#include "report_fragment.hpp"

namespace scope::reporting
{

bool ReportFragment::empty() const noexcept
{
    return textBody.empty() && markdownBody.empty() && jsonBody.empty() && csvRows.empty() &&
           htmlBody.empty();
}

} // namespace scope::reporting
