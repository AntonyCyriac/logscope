/**
 * @file random.cpp
 * @brief Random implementation.
 */

#include "random.hpp"

#include "error.hpp"

namespace scope::foundation
{

Random::Random(std::uint64_t seed) noexcept
    : m_engine(seed)
{
}

Random Random::create()
{
    std::random_device device;

    return Random(static_cast<std::uint64_t>(device()) << 32 |
                  static_cast<std::uint64_t>(device()));
}

std::uint32_t Random::nextUInt32()
{
    return static_cast<std::uint32_t>(m_engine());
}

std::uint64_t Random::nextUInt64()
{
    return m_engine();
}

Result<int> Random::nextInt(int minInclusive, int maxInclusive)
{
    if (minInclusive > maxInclusive)
    {
        return Result<int>(Error(ErrorCode::InvalidArgument, "Invalid random integer range."));
    }

    std::uniform_int_distribution<int> distribution(minInclusive, maxInclusive);

    return Result<int>(distribution(m_engine));
}

bool Random::nextBool()
{
    return (m_engine() & 1U) != 0U;
}

} // namespace scope::foundation
