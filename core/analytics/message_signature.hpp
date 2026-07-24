/**
 * @file message_signature.hpp
 * @brief Normalized message signatures for error clustering.
 */

#pragma once

#include <string>
#include <string_view>

namespace scope::analytics
{

/**
 * @brief Builds a cluster signature by normalizing digits and whitespace.
 */
[[nodiscard]] std::string normalizeClusterSignature(std::string_view message);

} // namespace scope::analytics
