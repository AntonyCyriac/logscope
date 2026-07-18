/**
 * @file service_registry_test.cpp
 * @brief Unit tests for ServiceRegistry.
 */

#include <gtest/gtest.h>

#include "runtime.hpp"

using scope::runtime::ServiceRegistry;

TEST(ServiceRegistryTest, RegisterService)
{
    ServiceRegistry& registry = ServiceRegistry::instance();

    registry.registerService("diagnostics");

    EXPECT_TRUE(registry.hasService("diagnostics"));
    EXPECT_FALSE(registry.hasService("unknown"));
}
