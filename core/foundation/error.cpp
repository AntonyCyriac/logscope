/**
 * @file error.cpp
 * @brief Implements the Error class.
 */

#include "error.hpp"

#include <utility>

namespace scope::foundation
{

Error::Error(ErrorCode code, std::string message)
    : m_code(code)
    , m_message(std::move(message))
{
}

Error::Error(const Error& other)
    : m_code(other.m_code)
    , m_message(other.m_message)
{
}

Error::Error(Error&& other) noexcept
    : m_code(other.m_code)
    , m_message(std::move(other.m_message))
{
}

Error& Error::operator=(const Error& other)
{
    if (this != &other)
    {
        m_code = other.m_code;
        m_message = other.m_message;
    }

    return *this;
}

Error& Error::operator=(Error&& other) noexcept
{
    if (this != &other)
    {
        m_code = other.m_code;
        m_message = std::move(other.m_message);
    }

    return *this;
}

ErrorCode Error::code() const noexcept
{
    return m_code;
}

const std::string& Error::message() const noexcept
{
    return m_message;
}

bool Error::hasError() const noexcept
{
    return m_code != ErrorCode::None;
}

} // namespace scope::foundation
