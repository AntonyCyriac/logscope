/**
 * @file configuration.cpp
 * @brief Configuration implementation.
 */

#include "configuration.hpp"

#include "diagnostics.hpp"
#include "log_macros.hpp"

#include "foundation/error.hpp"

namespace scope::runtime
{

void Configuration::set(std::string key, std::string value)
{
    SCOPE_LOG_DEBUG("runtime.config", "set key=" + key);

    m_values[std::move(key)] = std::move(value);
}

foundation::Result<std::string> Configuration::get(const std::string& key) const
{
    const auto iterator = m_values.find(key);

    if (iterator == m_values.end())
    {
        SCOPE_LOG_WARN("runtime.config", "missing key=" + key);

        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Configuration key not found."));
    }

    SCOPE_LOG_DEBUG("runtime.config", "get key=" + key);

    return foundation::Result<std::string>(iterator->second);
}

bool Configuration::has(const std::string& key) const noexcept
{
    return m_values.find(key) != m_values.end();
}

std::vector<std::string> Configuration::keys() const
{
    std::vector<std::string> keys;
    keys.reserve(m_values.size());

    for (const auto& entry : m_values)
    {
        keys.push_back(entry.first);
    }

    return keys;
}

} // namespace scope::runtime
