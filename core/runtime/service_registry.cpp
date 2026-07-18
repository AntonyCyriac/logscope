/**
 * @file service_registry.cpp
 * @brief ServiceRegistry implementation.
 */

#include "service_registry.hpp"

#include "log_macros.hpp"

namespace scope::runtime
{

ServiceRegistry& ServiceRegistry::instance()
{
    static ServiceRegistry registry;

    return registry;
}

void ServiceRegistry::registerService(std::string name)
{
    SCOPE_LOG_INFO("runtime.service", "register service=" + name);

    m_services.insert(std::move(name));
}

bool ServiceRegistry::hasService(const std::string& name) const noexcept
{
    return m_services.find(name) != m_services.end();
}

} // namespace scope::runtime
