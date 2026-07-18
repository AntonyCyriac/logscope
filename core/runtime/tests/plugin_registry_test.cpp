/**
 * @file plugin_registry_test.cpp
 * @brief Unit tests for PluginRegistry.
 */

#include <gtest/gtest.h>

#include "runtime.hpp"

using scope::runtime::PluginInfo;
using scope::runtime::PluginRegistry;

TEST(PluginRegistryTest, RegisterPlugin)
{
    PluginRegistry registry;

    registry.registerPlugin(PluginInfo{"parser.text", "1.0.0"});

    ASSERT_EQ(1U, registry.plugins().size());
    EXPECT_EQ("parser.text", registry.plugins()[0].name);
    EXPECT_EQ("1.0.0", registry.plugins()[0].version);
}
