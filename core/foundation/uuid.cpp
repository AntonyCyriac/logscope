/**
 * @file uuid.cpp
 * @brief UUID implementation.
 */

#include "uuid.hpp"

#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>

#include "error.hpp"

namespace scope::foundation
{

namespace
{

constexpr int UUID_LENGTH = 36;

constexpr int DASH1 = 8;
constexpr int DASH2 = 13;
constexpr int DASH3 = 18;
constexpr int DASH4 = 23;

bool isHexCharacter(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

std::uint8_t hexValue(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }

    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }

    return c - 'A' + 10;
}

} // namespace

Uuid::Uuid() noexcept
{
}

Uuid::Uuid(const std::array<std::uint8_t, 16>& bytes) noexcept
    : m_bytes(bytes)
{
}

Result<Uuid> Uuid::parse(const std::string& value)
{
    if (value.length() != UUID_LENGTH)
    {
        return Result<Uuid>(Error(ErrorCode::InvalidArgument, "Invalid UUID length."));
    }

    if (value[DASH1] != '-' || value[DASH2] != '-' || value[DASH3] != '-' || value[DASH4] != '-')
    {
        return Result<Uuid>(Error(ErrorCode::InvalidArgument, "Invalid UUID format."));
    }

    std::array<std::uint8_t, 16> bytes{};

    std::size_t index = 0;

    for (std::size_t i = 0; i < value.size();)
    {
        if (value[i] == '-')
        {
            ++i;
            continue;
        }

        if (!isHexCharacter(value[i]) || !isHexCharacter(value[i + 1]))
        {
            return Result<Uuid>(Error(ErrorCode::InvalidArgument, "Invalid UUID character."));
        }

        bytes[index++] = (hexValue(value[i]) << 4) | hexValue(value[i + 1]);

        i += 2;
    }

    return Result<Uuid>(Uuid(bytes));
}

Uuid Uuid::generate()
{
    std::random_device device;

    std::mt19937 generator(device());

    std::uniform_int_distribution<int> distribution(0, 255);

    std::array<std::uint8_t, 16> bytes{};

    for (auto& byte : bytes)
    {
        byte = static_cast<std::uint8_t>(distribution(generator));
    }

    bytes[6] = (bytes[6] & 0x0F) | 0x40;

    bytes[8] = (bytes[8] & 0x3F) | 0x80;

    return Uuid(bytes);
}

bool Uuid::isNil() const noexcept
{
    return std::all_of(m_bytes.begin(), m_bytes.end(),
                       [](std::uint8_t value) { return value == 0; });
}

std::string Uuid::toString() const
{
    std::ostringstream stream;

    stream << std::hex << std::setfill('0');

    for (std::size_t i = 0; i < m_bytes.size(); ++i)
    {
        if (i == 4 || i == 6 || i == 8 || i == 10)
        {
            stream << '-';
        }

        stream << std::setw(2) << static_cast<int>(m_bytes[i]);
    }

    return stream.str();
}

bool operator==(const Uuid& lhs, const Uuid& rhs) noexcept
{
    return lhs.m_bytes == rhs.m_bytes;
}

bool operator!=(const Uuid& lhs, const Uuid& rhs) noexcept
{
    return !(lhs == rhs);
}

bool operator<(const Uuid& lhs, const Uuid& rhs) noexcept
{
    return lhs.m_bytes < rhs.m_bytes;
}

} // namespace scope::foundation
