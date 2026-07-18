/**
 * @file random.hpp
 * @brief Pseudo-random number generator.
 */

#pragma once

#include <cstdint>
#include <random>

#include "result.hpp"

namespace scope::foundation
{

/**
 * @brief Generates pseudo-random numbers from a seedable engine.
 */
class Random
{
  public:
    /**
     * @brief Constructs a generator with an explicit seed.
     *
     * @param seed Engine seed.
     */
    explicit Random(std::uint64_t seed) noexcept;

    /**
     * @brief Creates a generator seeded from the system random device.
     */
    [[nodiscard]] static Random create();

    /**
     * @brief Returns the next 32-bit unsigned integer.
     */
    [[nodiscard]] std::uint32_t nextUInt32();

    /**
     * @brief Returns the next 64-bit unsigned integer.
     */
    [[nodiscard]] std::uint64_t nextUInt64();

    /**
     * @brief Returns the next integer in the inclusive range.
     *
     * @param minInclusive Minimum value.
     * @param maxInclusive Maximum value.
     * @return Random integer or error if the range is invalid.
     */
    [[nodiscard]] Result<int> nextInt(int minInclusive, int maxInclusive);

    /**
     * @brief Returns a random boolean value.
     */
    [[nodiscard]] bool nextBool();

  private:
    std::mt19937_64 m_engine;
};

} // namespace scope::foundation
