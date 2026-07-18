/**
 * @file configuration.hpp
 * @brief Key-value configuration store.
 */

#pragma once

#include <string>
#include <unordered_map>

#include "foundation/result.hpp"

namespace scope::runtime
{

/**
 * @brief Provides a simple key-value configuration store.
 */
class Configuration
{
  public:
    /**
     * @brief Sets a configuration value.
     *
     * @param key Configuration key.
     * @param value Configuration value.
     */
    void set(std::string key, std::string value);

    /**
     * @brief Returns a configuration value.
     *
     * @param key Configuration key.
     * @return Value or error if the key is not found.
     */
    [[nodiscard]] foundation::Result<std::string> get(const std::string& key) const;

    /**
     * @brief Determines whether a key exists.
     *
     * @param key Configuration key.
     * @return true if the key is present.
     */
    [[nodiscard]] bool has(const std::string& key) const noexcept;

  private:
    std::unordered_map<std::string, std::string> m_values;
};

} // namespace scope::runtime
