/**
 * @file configuration.cpp
 * @brief Configuration implementation.
 */

#include "configuration.hpp"

#include "foundation/error.hpp"

namespace scope::runtime
{

void Configuration::set(std::string key, std::string value)
{
    m_values[std::move(key)] = std::move(value);
}

foundation::Result<std::string> Configuration::get(const std::string& key) const
{
    const auto iterator = m_values.find(key);

    if (iterator == m_values.end())
    {
        return foundation::Result<std::string>(
            foundation::Error(foundation::ErrorCode::InvalidArgument, "Configuration key not found."));
    }

    return foundation::Result<std::string>(iterator->second);
}

bool Configuration::has(const std::string& key) const noexcept
{
    return m_values.find(key) != m_values.end();
}

} // namespace scope::runtime
