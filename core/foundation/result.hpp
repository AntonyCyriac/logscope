/**
 * @file result.hpp
 * @brief Defines the Result<T> type used throughout the Scope platform.
 */

#pragma once

#include "error.hpp"

#include <variant>

namespace scope::foundation
{

/**
 * @brief Represents the result of an operation.
 *
 * Result<T> contains either:
 * - a value of type T, or
 * - an Error describing why the operation failed.
 *
 * It provides a type-safe alternative to status codes and exceptions for
 * expected operational failures.
 *
 * @tparam T Value type.
 */
template <typename T> class Result
{
  public:
    /**
     * @brief Constructs a successful result.
     *
     * @param value Value returned by the operation.
     */
    explicit Result(const T& value);

    /**
     * @brief Constructs a successful result.
     *
     * @param value Value returned by the operation.
     */
    explicit Result(T&& value);

    /**
     * @brief Constructs a failed result.
     *
     * @param error Error describing the failure.
     */
    explicit Result(const Error& error);

    /**
     * @brief Constructs a failed result.
     *
     * @param error Error describing the failure.
     */
    explicit Result(Error&& error);

    /**
     * @brief Determines whether a value is present.
     *
     * @retval true Result contains a value.
     * @retval false Result contains an error.
     */
    bool hasValue() const noexcept;

    /**
     * @brief Determines whether an error is present.
     *
     * @retval true Result contains an error.
     * @retval false Result contains a value.
     */
    bool hasError() const noexcept;

    /**
     * @brief Returns whether the result is successful.
     *
     * @retval true Result contains a value.
     * @retval false Result contains an error.
     */
    explicit operator bool() const noexcept;

    /**
     * @brief Returns the stored value.
     *
     * @return Constant reference to the stored value.
     *
     * @throws std::logic_error if the result contains an error.
     */
    const T& value() const;

    /**
     * @brief Returns the stored value.
     *
     * @return Reference to the stored value.
     *
     * @throws std::logic_error if the result contains an error.
     */
    T& value();

    /**
     * @brief Returns the stored error.
     *
     * @return Constant reference to the stored error.
     *
     * @throws std::logic_error if the result contains a value.
     */
    const Error& error() const;

    /**
     * @brief Dereferences the stored value.
     *
     * @return Constant reference to the stored value.
     */
    const T& operator*() const;

    /**
     * @brief Dereferences the stored value.
     *
     * @return Reference to the stored value.
     */
    T& operator*();

    /**
     * @brief Returns a pointer to the stored value.
     *
     * @return Pointer to the stored value.
     */
    const T* operator->() const;

    /**
     * @brief Returns a pointer to the stored value.
     *
     * @return Pointer to the stored value.
     */
    T* operator->();

  private:
    /// Stores either a value or an error.
    std::variant<T, Error> m_result;
};

} // namespace scope::foundation

#include "result.inl"
