/**
 * @file result.inl
 * @brief Implements the Result<T> template.
 */

#pragma once

#include <stdexcept>
#include <utility>

namespace scope::foundation
{

template <typename T>
Result<T>::Result(const T& value)
    : m_result(value)
{
}

template <typename T>
Result<T>::Result(T&& value)
    : m_result(std::move(value))
{
}

template <typename T>
Result<T>::Result(const Error& error)
    : m_result(std::in_place_type<Error>, error)
{
}

template <typename T>
Result<T>::Result(Error&& error)
    : m_result(std::in_place_type<Error>, std::move(error))
{
}

template <typename T> bool Result<T>::hasValue() const noexcept
{
    return std::holds_alternative<T>(m_result);
}

template <typename T> bool Result<T>::hasError() const noexcept
{
    return !hasValue();
}

template <typename T> Result<T>::operator bool() const noexcept
{
    return hasValue();
}

template <typename T> const T& Result<T>::value() const
{
    if (hasError())
    {
        throw std::logic_error("Result does not contain a value.");
    }

    return std::get<T>(m_result);
}

template <typename T> T& Result<T>::value()
{
    if (hasError())
    {
        throw std::logic_error("Result does not contain a value.");
    }

    return std::get<T>(m_result);
}

template <typename T> const Error& Result<T>::error() const
{
    if (hasValue())
    {
        throw std::logic_error("Result does not contain an error.");
    }

    return std::get<Error>(m_result);
}

template <typename T> const T& Result<T>::operator*() const
{
    return value();
}

template <typename T> T& Result<T>::operator*()
{
    return value();
}

template <typename T> const T* Result<T>::operator->() const
{
    return &value();
}

template <typename T> T* Result<T>::operator->()
{
    return &value();
}

} // namespace scope::foundation
