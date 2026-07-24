/**
 * @file query_cache_codec.cpp
 */

#include "query_cache_codec.hpp"

#include <cstring>

namespace scope::storage
{

namespace
{

void appendUint64(std::string& output, const std::uint64_t value) noexcept
{
    const char bytes[sizeof(std::uint64_t)] = {
        static_cast<char>(value & 0xFFU),
        static_cast<char>((value >> 8U) & 0xFFU),
        static_cast<char>((value >> 16U) & 0xFFU),
        static_cast<char>((value >> 24U) & 0xFFU),
        static_cast<char>((value >> 32U) & 0xFFU),
        static_cast<char>((value >> 40U) & 0xFFU),
        static_cast<char>((value >> 48U) & 0xFFU),
        static_cast<char>((value >> 56U) & 0xFFU),
    };

    output.append(bytes, sizeof(bytes));
}

[[nodiscard]] std::uint64_t readUint64(const std::string& input, const std::size_t offset) noexcept
{
    std::uint64_t value = 0U;

    for (std::size_t index = 0U; index < sizeof(std::uint64_t); ++index)
    {
        value |= static_cast<std::uint64_t>(static_cast<unsigned char>(input[offset + index])) << (index * 8U);
    }

    return value;
}

} // namespace

std::string encodeLineNumbers(const std::vector<std::uint64_t>& lineNumbers) noexcept
{
    std::string encoded;
    encoded.reserve(lineNumbers.size() * sizeof(std::uint64_t));

    for (const std::uint64_t lineNumber : lineNumbers)
    {
        appendUint64(encoded, lineNumber);
    }

    return encoded;
}

std::vector<std::uint64_t> decodeLineNumbers(const std::string& blob) noexcept
{
    std::vector<std::uint64_t> lineNumbers;

    if (blob.empty() || blob.size() % sizeof(std::uint64_t) != 0U)
    {
        return lineNumbers;
    }

    lineNumbers.reserve(blob.size() / sizeof(std::uint64_t));

    for (std::size_t offset = 0U; offset < blob.size(); offset += sizeof(std::uint64_t))
    {
        lineNumbers.push_back(readUint64(blob, offset));
    }

    return lineNumbers;
}

} // namespace scope::storage
