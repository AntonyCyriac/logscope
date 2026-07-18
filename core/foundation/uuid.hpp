/**
 * @file uuid.hpp
 * @brief Universally Unique Identifier (UUID).
 *
 * Copyright (c) 2026 Scope Platform.
 */

#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "result.hpp"

namespace scope::foundation
{

/**
 * @brief Represents an RFC 4122 UUID.
 *
 * A UUID is an immutable value object that stores its data
 * internally as sixteen bytes.
 *
 * The default constructor creates the Nil UUID:
 *
 * 00000000-0000-0000-0000-000000000000
 *
 * Parsing and random generation are provided by static
 * factory methods on this class.
 */
class Uuid
{
  public:
    /**
     * @brief Constructs the Nil UUID.
     */
    Uuid() noexcept;

    /**
     * @brief Parses a UUID string.
     *
     * @param value UUID string.
     *
     * @return Parsed UUID or Error.
     */
    static Result<Uuid> parse(const std::string& value);

    /**
     * @brief Generates a Version 4 UUID.
     *
     * @return Random UUID.
     */
    static Uuid generate();

    /**
     * @brief Determines whether this UUID is Nil.
     *
     * @return true if every byte is zero.
     */
    bool isNil() const noexcept;

    /**
     * @brief Converts the UUID to canonical string format.
     *
     * Example:
     *
     * 550e8400-e29b-41d4-a716-446655440000
     *
     * @return UUID string.
     */
    std::string toString() const;

    friend bool operator==(const Uuid& lhs, const Uuid& rhs) noexcept;

    friend bool operator!=(const Uuid& lhs, const Uuid& rhs) noexcept;

    friend bool operator<(const Uuid& lhs, const Uuid& rhs) noexcept;

  private:
    explicit Uuid(const std::array<std::uint8_t, 16>& bytes) noexcept;

  private:
    std::array<std::uint8_t, 16> m_bytes{};
};

} // namespace scope::foundation
