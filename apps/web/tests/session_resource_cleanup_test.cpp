/**
 * @file session_resource_cleanup_test.cpp
 */

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#include "session_resource_cleanup.hpp"
#include "session_store.hpp"

namespace
{

std::filesystem::path writeTempFile(const std::string& suffix)
{
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / ("logscope-cleanup-test-" + suffix + ".log");

    std::ofstream stream(path, std::ios::binary);
    stream << "line\n";

    return path;
}

} // namespace

TEST(SessionResourceCleanupTest, RemoveTempUploadFileDeletesDiskFile)
{
    scope::web::WorkspaceSession session;
    session.service = std::make_unique<scope::application::ApplicationService>();
    const std::filesystem::path tempPath = writeTempFile("remove");
    session.tempUploadPath = tempPath.string();

    scope::web::removeTempUploadFile(session);

    EXPECT_TRUE(session.tempUploadPath.empty());
    EXPECT_FALSE(std::filesystem::exists(tempPath));
}

TEST(SessionResourceCleanupTest, ReplaceStagedUploadRemovesSupersededFile)
{
    scope::web::WorkspaceSession session;
    session.service = std::make_unique<scope::application::ApplicationService>();
    const std::filesystem::path firstPath = writeTempFile("first");
    const std::filesystem::path secondPath = writeTempFile("second");
    session.tempUploadPath = firstPath.string();

    scope::web::replaceStagedUpload(session, secondPath.string());

    EXPECT_EQ(secondPath.string(), session.tempUploadPath);
    EXPECT_FALSE(std::filesystem::exists(firstPath));
    EXPECT_TRUE(std::filesystem::exists(secondPath));

    std::error_code errorCode;
    std::filesystem::remove(secondPath, errorCode);
}

TEST(SessionResourceCleanupTest, CleanupSessionResourcesRemovesTempFile)
{
    scope::web::WorkspaceSession session;
    session.service = std::make_unique<scope::application::ApplicationService>();
    const std::filesystem::path tempPath = writeTempFile("cleanup");
    session.tempUploadPath = tempPath.string();

    scope::web::cleanupSessionResources(session);

    EXPECT_TRUE(session.tempUploadPath.empty());
    EXPECT_FALSE(std::filesystem::exists(tempPath));
}
