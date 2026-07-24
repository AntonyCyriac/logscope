/**
 * @file storage_config_test.cpp
 */

#include <gtest/gtest.h>

#include "runtime/configuration.hpp"
#include "storage_config.hpp"

using scope::runtime::Configuration;
using scope::storage::StorageConfig;
using scope::storage::StorageMode;
using scope::storage::parseStorageMode;
using scope::storage::resolveStorageConfig;
using scope::storage::storageModeName;
using scope::storage::validateStorageConfiguration;

TEST(StorageConfigTest, ParsesStorageModes)
{
    EXPECT_EQ(StorageMode::Memory, parseStorageMode("memory"));
    EXPECT_EQ(StorageMode::Hybrid, parseStorageMode("hybrid"));
    EXPECT_EQ(StorageMode::Persistent, parseStorageMode("persistent"));
    EXPECT_FALSE(parseStorageMode("invalid").has_value());
}

TEST(StorageConfigTest, ResolvesPersistIndexOverride)
{
    Configuration configuration;
    StorageConfig cliOverrides;
    cliOverrides.persistIndex = true;

    const StorageConfig resolved = resolveStorageConfig(configuration, cliOverrides);

    EXPECT_TRUE(resolved.persistIndex);
    EXPECT_EQ(StorageMode::Hybrid, resolved.mode);
    EXPECT_TRUE(resolved.usesPersistentStore());
}

TEST(StorageConfigTest, ResolvesIndexPathOverride)
{
    Configuration configuration;
    StorageConfig cliOverrides;
    cliOverrides.indexPath = scope::foundation::Path("custom.index.db");

    const StorageConfig resolved = resolveStorageConfig(configuration, cliOverrides);

    EXPECT_EQ(StorageMode::Persistent, resolved.mode);
    ASSERT_TRUE(resolved.indexPath.has_value());
    EXPECT_EQ("custom.index.db", resolved.indexPath->string());
}

TEST(StorageConfigTest, ValidatesStorageModeKey)
{
    Configuration configuration;
    configuration.set("storage.mode", "hybrid");

    const auto result = validateStorageConfiguration(configuration);

    ASSERT_TRUE(result);
}

TEST(StorageConfigTest, RejectsInvalidStorageMode)
{
    Configuration configuration;
    configuration.set("storage.mode", "cloud");

    const auto result = validateStorageConfiguration(configuration);

    EXPECT_FALSE(result);
}

TEST(StorageConfigTest, RejectsInvalidSpillThreshold)
{
    Configuration configuration;
    configuration.set("storage.spill_threshold", "not-a-number");

    const auto result = validateStorageConfiguration(configuration);

    EXPECT_FALSE(result);
}

TEST(StorageConfigTest, ExposesModeNames)
{
    EXPECT_EQ("memory", storageModeName(StorageMode::Memory));
    EXPECT_EQ("hybrid", storageModeName(StorageMode::Hybrid));
    EXPECT_EQ("persistent", storageModeName(StorageMode::Persistent));
}
