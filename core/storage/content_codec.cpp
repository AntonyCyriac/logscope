/**
 * @file content_codec.cpp
 */

#include "content_codec.hpp"

#include <zlib.h>

#include "foundation/error.hpp"

namespace scope::storage
{

foundation::Result<std::string> compressZlib(const std::string_view input)
{
    const uLong sourceLength = static_cast<uLong>(input.size());
    const uLong bound = compressBound(sourceLength);

    std::string output;
    output.resize(static_cast<std::size_t>(bound));

    uLongf destinationLength = bound;

    const int result =
        compress2(reinterpret_cast<Bytef*>(output.data()), &destinationLength,
                  reinterpret_cast<const Bytef*>(input.data()), sourceLength, Z_DEFAULT_COMPRESSION);

    if (result != Z_OK)
    {
        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::IOError, "zlib compression failed."));
    }

    output.resize(static_cast<std::size_t>(destinationLength));

    return foundation::Result<std::string>(std::move(output));
}

foundation::Result<std::string> decompressZlib(const void* data, const std::size_t size)
{
    if (data == nullptr || size == 0U)
    {
        return foundation::Result<std::string>(std::string{});
    }

    std::size_t capacity = size * 4U;

    if (capacity < 256U)
    {
        capacity = 256U;
    }

    for (int attempt = 0; attempt < 8; ++attempt)
    {
        std::string output;
        output.resize(capacity);

        uLongf destinationLength = static_cast<uLongf>(capacity);

        const int result = uncompress(
            reinterpret_cast<Bytef*>(output.data()), &destinationLength,
            reinterpret_cast<const Bytef*>(data), static_cast<uLong>(size));

        if (result == Z_OK)
        {
            output.resize(static_cast<std::size_t>(destinationLength));

            return foundation::Result<std::string>(std::move(output));
        }

        if (result != Z_BUF_ERROR)
        {
            return foundation::Result<std::string>(foundation::Error(
                foundation::ErrorCode::ParseError, "zlib decompression failed."));
        }

        capacity *= 2U;
    }

    return foundation::Result<std::string>(foundation::Error(
        foundation::ErrorCode::ParseError, "zlib decompression buffer overflow."));
}

} // namespace scope::storage
