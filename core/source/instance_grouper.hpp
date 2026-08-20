/**
 * @file instance_grouper.hpp
 * @brief Assigns instance keys to discovered paths.
 */

#pragma once

#include <string>
#include <string_view>

namespace scope::source
{

[[nodiscard]] std::string deriveInstanceKey(std::string_view relativePath) noexcept;

} // namespace scope::source
