/**
 * @file query_value.cpp
 */

#include "query_value.hpp"

#include <utility>

namespace scope::query
{

QueryValue::QueryValue(const Kind kind) : m_kind(kind) {}

QueryValue::QueryValue(const QueryValue& other)
    : m_kind(other.m_kind)
    , m_string(other.m_string)
    , m_number(other.m_number)
    , m_level(other.m_level)
{
}

QueryValue::QueryValue(QueryValue&& other) noexcept
    : m_kind(other.m_kind)
    , m_string(std::move(other.m_string))
    , m_number(other.m_number)
    , m_level(other.m_level)
{
}

QueryValue& QueryValue::operator=(const QueryValue& other)
{
    if (this != &other)
    {
        m_kind = other.m_kind;
        m_string = other.m_string;
        m_number = other.m_number;
        m_level = other.m_level;
    }

    return *this;
}

QueryValue& QueryValue::operator=(QueryValue&& other) noexcept
{
    if (this != &other)
    {
        m_kind = other.m_kind;
        m_string = std::move(other.m_string);
        m_number = other.m_number;
        m_level = other.m_level;
    }

    return *this;
}

QueryValue QueryValue::stringValue(std::string value)
{
    QueryValue result(Kind::String);
    result.m_string = std::move(value);

    return result;
}

QueryValue QueryValue::numberValue(const std::uint64_t value)
{
    QueryValue result(Kind::Number);
    result.m_number = value;

    return result;
}

QueryValue QueryValue::levelValue(const analysis::DetectedLogLevel value)
{
    QueryValue result(Kind::Level);
    result.m_level = value;

    return result;
}

QueryValue::Kind QueryValue::kind() const noexcept
{
    return m_kind;
}

const std::string& QueryValue::stringValue() const noexcept
{
    return m_string;
}

std::uint64_t QueryValue::numberValue() const noexcept
{
    return m_number;
}

analysis::DetectedLogLevel QueryValue::levelValue() const noexcept
{
    return m_level;
}

} // namespace scope::query
