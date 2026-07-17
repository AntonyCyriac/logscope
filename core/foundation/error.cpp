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
