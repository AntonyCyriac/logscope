/**
 * @file error.hpp
 * @brief Defines the Error and ErrorCode types used throughout the Scope platform.
 */

#pragma once

#include <string>

namespace scope::foundation
{

/**
 * @brief Represents a category of error.
 *
 * ErrorCode provides a lightweight classification for errors that may
 * occur throughout the Scope platform. It is intended to be used in
 * conjunction with the Error class to provide structured error reporting.
 */
enum class ErrorCode
{
    None = 0,        ///< No error occurred.
    Unknown,         ///< An unknown error occurred.
    InvalidArgument, ///< An invalid argument was supplied.
    FileNotFound,    ///< A requested file could not be found.
    IOError,         ///< An input/output operation failed.
    ParseError       ///< Data parsing failed.
};

/**
 * @brief Represents an error returned by an operation.
 *
 * The Error class encapsulates an error code together with a descriptive
 * message. It provides a simple and consistent mechanism for reporting
 * failures throughout the Scope platform.
 *
 * More advanced capabilities, such as source locations or stack traces,
 * may be introduced in future revisions if required.
 */
class Error
{
  public:
    /**
     * @brief Constructs an object representing no error.
     */
    Error() = default;

    /**
     * @brief Constructs an error.
     *
     * @param code Error classification.
     * @param message Human-readable description of the error.
     */
    Error(ErrorCode code, std::string message);

    /**
     * @brief Returns the error code.
     *
     * @return The associated ErrorCode.
     */
    ErrorCode code() const noexcept;

    /**
     * @brief Returns the error message.
     *
     * @return A constant reference to the descriptive message.
     */
    const std::string& message() const noexcept;

    /**
     * @brief Determines whether an error is present.
     *
     * @retval true An error exists.
     * @retval false No error is present.
     */
    bool hasError() const noexcept;

  private:
    /// Error classification.
    ErrorCode m_code{ErrorCode::None};

    /// Human-readable error description.
    std::string m_message;
};

} // namespace scope::foundation
