/**
 * @file service_registry.hpp
 * @brief Service registration.
 */

#pragma once

#include <string>
#include <unordered_set>

namespace scope::runtime
{

/**
 * @brief Maintains registered platform service names.
 */
class ServiceRegistry
{
  public:
    /**
     * @brief Returns the global service registry.
     */
    static ServiceRegistry& instance();

    /**
     * @brief Registers a service by name.
     *
     * @param name Service name.
     */
    void registerService(std::string name);

    /**
     * @brief Determines whether a service is registered.
     *
     * @param name Service name.
     * @return true if registered.
     */
    [[nodiscard]] bool hasService(const std::string& name) const noexcept;

  private:
    ServiceRegistry() = default;

    std::unordered_set<std::string> m_services;
};

} // namespace scope::runtime
