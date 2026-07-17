/**
 * @file status.hpp
 * @brief Defines the Status type used throughout the Scope platform.
 */

#pragma once

namespace scope::foundation
{

/**
 * @brief Represents the outcome of an operation.
 *
 * This enumeration provides a lightweight status code that can be
 * returned by functions throughout the Scope platform.
 *
 * More detailed error information will be introduced later through
 * dedicated Error and Result<T> types.
 */
enum class Status
{
    Success = 0,
    Failure
};

} // namespace scope::foundation
