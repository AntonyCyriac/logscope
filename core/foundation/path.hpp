/**
 * @file path.hpp
 * @brief Filesystem path value type.
 */

#pragma once

#include <string>
#include <string_view>

namespace scope::foundation
{

/**
 * @brief Represents a filesystem path.
 */
class Path
{
  public:
    /**
     * @brief Constructs an empty path.
     */
    Path() noexcept;

    /**
     * @brief Constructs a path from a string.
     *
     * @param value Path string.
     */
    explicit Path(std::string_view value);

    /**
     * @brief Returns the path as a native string.
     */
    [[nodiscard]] const std::string& string() const noexcept;

    /**
     * @brief Determines whether the path is empty.
     */
    [[nodiscard]] bool isEmpty() const noexcept;

    /**
     * @brief Determines whether the path is absolute.
     */
    [[nodiscard]] bool isAbsolute() const;

    /**
     * @brief Returns the parent directory path.
     */
    [[nodiscard]] Path parent() const;

    /**
     * @brief Returns the filename component.
     */
    [[nodiscard]] Path filename() const;

    /**
     * @brief Returns the file extension (including dot).
     */
    [[nodiscard]] Path extension() const;

    /**
     * @brief Appends a path component.
     *
     * @param other Path component to append.
     * @return Combined path.
     */
    [[nodiscard]] Path append(std::string_view other) const;

    friend bool operator==(const Path& lhs, const Path& rhs) noexcept;

    friend bool operator!=(const Path& lhs, const Path& rhs) noexcept;

  private:
    std::string m_path;
};

} // namespace scope::foundation
