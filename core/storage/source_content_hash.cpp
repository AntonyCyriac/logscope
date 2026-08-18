/**
 * @file source_content_hash.cpp
 */

#include "source_content_hash.hpp"

#include <array>
#include <cstdio>
#include <fstream>
#include <vector>

#include "foundation/error.hpp"

namespace scope::storage
{

namespace
{

constexpr std::size_t kSha256BlockSize = 64U;
constexpr std::size_t kSha256DigestSize = 32U;
constexpr std::size_t kReadBufferSize = 65536U;

struct Sha256Context
{
    std::uint32_t state[8];
    std::uint64_t bitLength{0U};
    std::array<std::uint8_t, kSha256BlockSize> buffer{};
    std::size_t bufferLength{0U};
};

[[nodiscard]] constexpr std::uint32_t rotr(const std::uint32_t value, const std::uint32_t bits) noexcept
{
    return (value >> bits) | (value << (32U - bits));
}

constexpr std::array<std::uint32_t, 64> kSha256Constants{{
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU, 0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U, 0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U,
}};

void sha256Transform(Sha256Context& context, const std::uint8_t* block)
{
    std::uint32_t words[64];

    for (std::size_t index = 0U; index < 16U; ++index)
    {
        const std::size_t offset = index * 4U;
        words[index] = (static_cast<std::uint32_t>(block[offset]) << 24U) |
                       (static_cast<std::uint32_t>(block[offset + 1U]) << 16U) |
                       (static_cast<std::uint32_t>(block[offset + 2U]) << 8U) |
                       static_cast<std::uint32_t>(block[offset + 3U]);
    }

    for (std::size_t index = 16U; index < 64U; ++index)
    {
        const std::uint32_t sigma0 = rotr(words[index - 15U], 7U) ^ rotr(words[index - 15U], 18U) ^
                                     (words[index - 15U] >> 3U);
        const std::uint32_t sigma1 = rotr(words[index - 2U], 17U) ^ rotr(words[index - 2U], 19U) ^
                                     (words[index - 2U] >> 10U);
        words[index] = words[index - 16U] + sigma0 + words[index - 7U] + sigma1;
    }

    std::uint32_t a = context.state[0];
    std::uint32_t b = context.state[1];
    std::uint32_t c = context.state[2];
    std::uint32_t d = context.state[3];
    std::uint32_t e = context.state[4];
    std::uint32_t f = context.state[5];
    std::uint32_t g = context.state[6];
    std::uint32_t h = context.state[7];

    for (std::size_t index = 0U; index < 64U; ++index)
    {
        const std::uint32_t sigma1 = rotr(e, 6U) ^ rotr(e, 11U) ^ rotr(e, 25U);
        const std::uint32_t ch = (e & f) ^ ((~e) & g);
        const std::uint32_t temp1 = h + sigma1 + ch + kSha256Constants[index] + words[index];
        const std::uint32_t sigma0 = rotr(a, 2U) ^ rotr(a, 13U) ^ rotr(a, 22U);
        const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const std::uint32_t temp2 = sigma0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    context.state[0] += a;
    context.state[1] += b;
    context.state[2] += c;
    context.state[3] += d;
    context.state[4] += e;
    context.state[5] += f;
    context.state[6] += g;
    context.state[7] += h;
}

void sha256Init(Sha256Context& context) noexcept
{
    context.state[0] = 0x6a09e667U;
    context.state[1] = 0xbb67ae85U;
    context.state[2] = 0x3c6ef372U;
    context.state[3] = 0xa54ff53aU;
    context.state[4] = 0x510e527fU;
    context.state[5] = 0x9b05688cU;
    context.state[6] = 0x1f83d9abU;
    context.state[7] = 0x5be0cd19U;
    context.bitLength = 0U;
    context.bufferLength = 0U;
}

void sha256Update(Sha256Context& context, const std::uint8_t* data, std::size_t length)
{
    context.bitLength += static_cast<std::uint64_t>(length) * 8U;

    while (length > 0U)
    {
        const std::size_t space = kSha256BlockSize - context.bufferLength;
        const std::size_t copyLength = length < space ? length : space;

        for (std::size_t index = 0U; index < copyLength; ++index)
        {
            context.buffer[context.bufferLength + index] = data[index];
        }

        context.bufferLength += copyLength;
        data += copyLength;
        length -= copyLength;

        if (context.bufferLength == kSha256BlockSize)
        {
            sha256Transform(context, context.buffer.data());
            context.bufferLength = 0U;
        }
    }
}

void sha256Final(Sha256Context& context, std::array<std::uint8_t, kSha256DigestSize>& digest)
{
    context.buffer[context.bufferLength++] = 0x80U;

    if (context.bufferLength > 56U)
    {
        while (context.bufferLength < kSha256BlockSize)
        {
            context.buffer[context.bufferLength++] = 0U;
        }

        sha256Transform(context, context.buffer.data());
        context.bufferLength = 0U;
    }

    while (context.bufferLength < 56U)
    {
        context.buffer[context.bufferLength++] = 0U;
    }

    for (int index = 7; index >= 0; --index)
    {
        context.buffer[context.bufferLength++] = static_cast<std::uint8_t>((context.bitLength >> (index * 8U)) & 0xFFU);
    }

    sha256Transform(context, context.buffer.data());

    for (std::size_t index = 0U; index < 8U; ++index)
    {
        digest[index * 4U] = static_cast<std::uint8_t>((context.state[index] >> 24U) & 0xFFU);
        digest[index * 4U + 1U] = static_cast<std::uint8_t>((context.state[index] >> 16U) & 0xFFU);
        digest[index * 4U + 2U] = static_cast<std::uint8_t>((context.state[index] >> 8U) & 0xFFU);
        digest[index * 4U + 3U] = static_cast<std::uint8_t>(context.state[index] & 0xFFU);
    }
}

[[nodiscard]] std::string digestToHex(const std::array<std::uint8_t, kSha256DigestSize>& digest)
{
    static constexpr char kHexDigits[] = "0123456789abcdef";

    std::string hex;
    hex.resize(kSha256DigestSize * 2U);

    for (std::size_t index = 0U; index < kSha256DigestSize; ++index)
    {
        hex[index * 2U] = kHexDigits[(digest[index] >> 4U) & 0x0FU];
        hex[index * 2U + 1U] = kHexDigits[digest[index] & 0x0FU];
    }

    return hex;
}

[[nodiscard]] foundation::Result<std::string> hashStream(std::ifstream& input, const std::uint64_t maxBytes)
{
    Sha256Context context;
    sha256Init(context);

    std::vector<char> buffer(kReadBufferSize);
    std::uint64_t remaining = maxBytes;

    while (remaining > 0U && input)
    {
        const std::size_t chunkSize =
            static_cast<std::size_t>(remaining < kReadBufferSize ? remaining : kReadBufferSize);

        input.read(buffer.data(), static_cast<std::streamsize>(chunkSize));

        if (input.bad())
        {
            return foundation::Result<std::string>(foundation::Error(
                foundation::ErrorCode::IOError, "Failed to read source file while computing content hash."));
        }

        const std::streamsize bytesRead = input.gcount();

        if (bytesRead <= 0)
        {
            break;
        }

        sha256Update(context, reinterpret_cast<const std::uint8_t*>(buffer.data()),
                     static_cast<std::size_t>(bytesRead));
        remaining -= static_cast<std::uint64_t>(bytesRead);
    }

    if (maxBytes > 0U && remaining > 0U)
    {
        return foundation::Result<std::string>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            "Source file is shorter than expected while computing prefix content hash."));
    }

    std::array<std::uint8_t, kSha256DigestSize> digest{};
    sha256Final(context, digest);

    return foundation::Result<std::string>(digestToHex(digest));
}

} // namespace

foundation::Result<std::string> computeSourceSha256Hex(const foundation::Path& path)
{
    std::ifstream input(path.string(), std::ios::binary);

    if (!input.is_open())
    {
        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to open source file for content hash."));
    }

    input.seekg(0, std::ios::end);

    if (!input)
    {
        return foundation::Result<std::string>(foundation::Error(
            foundation::ErrorCode::IOError, "Failed to determine source file size for content hash."));
    }

    const auto fileSize = static_cast<std::uint64_t>(input.tellg());
    input.seekg(0, std::ios::beg);

    return hashStream(input, fileSize);
}

foundation::Result<std::string> computeSourcePrefixSha256Hex(const foundation::Path& path,
                                                               const std::uint64_t byteLength)
{
    std::ifstream input(path.string(), std::ios::binary);

    if (!input.is_open())
    {
        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::IOError, "Failed to open source file for prefix content hash."));
    }

    return hashStream(input, byteLength);
}

} // namespace scope::storage
