/**
 * @file report_writer.cpp
 * @brief Writes generated reports to files or streams.
 */

#include "report_writer.hpp"

#include <filesystem>
#include <fstream>

#include "foundation/error.hpp"
#include "report_format.hpp"

namespace scope::cli
{

foundation::Result<bool> writeReport(const reporting::Report& report,
                                     const std::optional<foundation::Path>& outputFile,
                                     std::ostream& output,
                                     std::ostream& errorOutput)
{
    if (outputFile)
    {
        const std::filesystem::path path = outputFile->string();
        std::error_code errorCode;

        if (path.has_parent_path())
        {
            std::filesystem::create_directories(path.parent_path(), errorCode);

            if (errorCode)
            {
                return foundation::Result<bool>(foundation::Error(
                    foundation::ErrorCode::IOError,
                    "Failed to create output directory: " + errorCode.message()));
            }
        }

        if (report.isBinary())
        {
            std::ofstream stream(path, std::ios::binary);

            if (!stream)
            {
                return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::IOError,
                                                                  "Failed to open output file for writing."));
            }

            stream.write(reinterpret_cast<const char*>(report.bytes().data()),
                         static_cast<std::streamsize>(report.bytes().size()));

            if (!stream)
            {
                return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::IOError,
                                                                  "Failed to write binary report."));
            }
        }
        else
        {
            std::ofstream stream(path);

            if (!stream)
            {
                return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::IOError,
                                                                  "Failed to open output file for writing."));
            }

            stream << report.text();

            if (!stream)
            {
                return foundation::Result<bool>(foundation::Error(foundation::ErrorCode::IOError,
                                                                  "Failed to write report."));
            }
        }

        return foundation::Result<bool>(true);
    }

    if (report.isBinary())
    {
        errorOutput << "Binary report format cannot be written to stdout. Use --output <file>.\n";

        return foundation::Result<bool>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, "Binary report requires --output <file>."));
    }

    output << report.text() << std::endl;

    return foundation::Result<bool>(true);
}

} // namespace scope::cli
