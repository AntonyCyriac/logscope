/**
 * @file json_parse.hpp
 * @brief Minimal JSON field extraction for REST handlers (M15.1).
 */

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace scope::web
{

[[nodiscard]] std::optional<std::string> jsonStringField(std::string_view body, std::string_view key);

[[nodiscard]] std::optional<bool> jsonBoolField(std::string_view body, std::string_view key);

[[nodiscard]] std::optional<std::int64_t> jsonIntField(std::string_view body, std::string_view key);

} // namespace scope::web
