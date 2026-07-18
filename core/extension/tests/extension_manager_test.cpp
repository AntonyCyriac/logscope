/**
 * @file extension_manager_test.cpp
 * @brief Unit tests for ExtensionManager.
 */

#include <gtest/gtest.h>

#include "extension.hpp"
#include "foundation.hpp"

using scope::extension::ExtensionDescriptor;
using scope::extension::ExtensionManager;
using scope::extension::ExtensionStatus;
using scope::foundation::ErrorCode;
using scope::runtime::Configuration;

namespace
{

scope::foundation::Result<bool> failingInitialize()
{
    return scope::foundation::Result<bool>(
        scope::foundation::Error(scope::foundation::ErrorCode::Unknown, "Initialization failed."));
}

} // namespace

TEST(ExtensionManagerTest, RegistersBuiltInExtensions)
{
    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    const auto extensions = manager.listExtensions();

    ASSERT_EQ(3U, extensions.size());
    EXPECT_EQ("analysis.log-levels", extensions[0].id);
    EXPECT_EQ("source.files", extensions[1].id);
    EXPECT_EQ("reporting.multi-format", extensions[2].id);
}

TEST(ExtensionManagerTest, DescribeReturnsExtensionMetadata)
{
    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    const auto infoResult = manager.describe("source.files");

    ASSERT_TRUE(infoResult.hasValue());
    EXPECT_EQ("1.0.0", infoResult->version);
    EXPECT_TRUE(infoResult->enabled);
    EXPECT_EQ(ExtensionStatus::Ready, infoResult->status);
    EXPECT_NE(std::string::npos, infoResult->description.find("stdin"));
}

TEST(ExtensionManagerTest, DescribeMissingExtensionReturnsError)
{
    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    const auto infoResult = manager.describe("missing.extension");

    ASSERT_TRUE(infoResult.hasError());
    EXPECT_EQ(ErrorCode::InvalidArgument, infoResult.error().code());
}

TEST(ExtensionManagerTest, AppliesConfigurationEnablement)
{
    ExtensionManager manager = ExtensionManager::createWithBuiltIns();

    Configuration configuration;
    configuration.set("extensions.reporting.multi-format.enabled", "false");

    manager.applyConfiguration(configuration);

    const auto infoResult = manager.describe("reporting.multi-format");

    ASSERT_TRUE(infoResult.hasValue());
    EXPECT_FALSE(infoResult->enabled);
    EXPECT_EQ(ExtensionStatus::Disabled, infoResult->status);
}

TEST(ExtensionManagerTest, InitializationFailureDoesNotAbortManager)
{
    ExtensionManager manager;
    manager.registerBuiltIn(
        ExtensionDescriptor{"test.failing", "1.0.0", "Test extension.", failingInitialize, true});

    manager.initializeEnabled();

    const auto infoResult = manager.describe("test.failing");

    ASSERT_TRUE(infoResult.hasValue());
    EXPECT_EQ(ExtensionStatus::InitializationFailed, infoResult->status);
}
