/**
 * @file string.hpp
 * @brief String utility functions.
 */

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace scope::foundation
{

/**
 * @brief Determines whether a string is empty or contains only whitespace.
 */
[[nodiscard]] bool isBlank(std::string_view value) noexcept;

/**
 * @brief Removes leading and trailing whitespace.
 */
[[nodiscard]] std::string trim(std::string_view value);

/**
 * @brief Removes leading whitespace.
 */
[[nodiscard]] std::string trimLeft(std::string_view value);

/**
 * @brief Removes trailing whitespace.
 */
[[nodiscard]] std::string trimRight(std::string_view value);

/**
 * @brief Converts a string to lowercase.
 */
[[nodiscard]] std::string toLower(std::string_view value);

/**
 * @brief Converts a string to uppercase.
 */
[[nodiscard]] std::string toUpper(std::string_view value);

/**
 * @brief Determines whether a string starts with a prefix.
 */
[[nodiscard]] bool startsWith(std::string_view value, std::string_view prefix) noexcept;

/**
 * @brief Determines whether a string ends with a suffix.
 */
[[nodiscard]] bool endsWith(std::string_view value, std::string_view suffix) noexcept;

/**
 * @brief Splits a string by a delimiter.
 */
[[nodiscard]] std::vector<std::string> split(std::string_view value, char delimiter);

} // namespace scope::foundation
