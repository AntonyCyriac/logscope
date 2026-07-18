/**
 * @file report_output.cpp
 * @brief CLI report formatting implementation.
 */

#include "report_output.hpp"

#include <sstream>

#include "reporting.hpp"

namespace scope::cli
{

namespace
{

std::string escapeJsonString(std::string_view value)
{
    std::ostringstream output;

    output << '"';

    for (const char character : value)
    {
        switch (character)
        {
        case '"':
            output << "\\\"";
            break;
        case '\\':
            output << "\\\\";
            break;
        case '\n':
            output << "\\n";
            break;
        case '\r':
            output << "\\r";
            break;
        case '\t':
            output << "\\t";
            break;
        default:
            output << character;
            break;
        }
    }

    output << '"';

    return output.str();
}

} // namespace

std::string formatAnalysisOutput(const analysis::AnalysisModel& model, const OutputFormat format)
{
    if (format == OutputFormat::Json)
    {
        std::ostringstream output;

        output << "{\n"
               << "  \"source\": " << escapeJsonString(model.sourcePath().string()) << ",\n"
               << "  \"totalLines\": " << model.totalLines() << "\n"
               << '}';

        return output.str();
    }

    const reporting::Report report = reporting::ReportGenerator{}.generate(model);

    return report.text();
}

} // namespace scope::cli
