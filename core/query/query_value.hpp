/**
 * @file query_value.hpp
 * @brief Literal values in filter query expressions.
 */

#pragma once

#include <cstdint>
#include <string>

#include "log_line_classifier.hpp"

namespace scope::query
{

/**
 * @brief Typed literal used in comparisons.
 */
class QueryValue
{
  public:
    enum class Kind
    {
        String,
        Number,
        Level
    };

    [[nodiscard]] static QueryValue stringValue(std::string value);

    [[nodiscard]] static QueryValue numberValue(std::uint64_t value);

    [[nodiscard]] static QueryValue levelValue(analysis::DetectedLogLevel value);

    [[nodiscard]] Kind kind() const noexcept;

    [[nodiscard]] const std::string& stringValue() const noexcept;

    [[nodiscard]] std::uint64_t numberValue() const noexcept;

    [[nodiscard]] analysis::DetectedLogLevel levelValue() const noexcept;

  private:
    explicit QueryValue(Kind kind);

    Kind m_kind{Kind::String};
    std::string m_string;
    std::uint64_t m_number{0U};
    analysis::DetectedLogLevel m_level{analysis::DetectedLogLevel::Other};
};

} // namespace scope::query
