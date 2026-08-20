/**
 * @file text_sniff.cpp
 * @brief Text/binary head sniff implementation.
 */

#include "text_sniff.hpp"

#include <fstream>

namespace scope::source
{

namespace
{

[[nodiscard]] bool containsBinaryContent(const std::string_view text) noexcept
{
    if (text.empty())
    {
        return false;
    }

    std::size_t controlCount = 0U;

    for (const unsigned char character : text)
    {
        if (character == '\n' || character == '\r' || character == '\t')
        {
            continue;
        }

        if (character < 0x20U || character == 0x7FU)
        {
            ++controlCount;
        }
    }

    return controlCount * 4U > text.size();
}

} // namespace

foundation::Result<HeadSniffResult> sniffFileHead(const foundation::Path& path)
{
    std::ifstream stream(path.string(), std::ios::binary);

    if (!stream)
    {
        return foundation::Result<HeadSniffResult>(
            foundation::Error(foundation::ErrorCode::IOError, "Unable to read file head for discovery."));
    }

    std::string buffer(8192U, '\0');
    stream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    buffer.resize(static_cast<std::size_t>(stream.gcount()));

    if (containsBinaryContent(buffer))
    {
        return foundation::Result<HeadSniffResult>(HeadSniffResult::BinaryCandidate);
    }

    return foundation::Result<HeadSniffResult>(HeadSniffResult::TextCandidate);
}

} // namespace scope::source
