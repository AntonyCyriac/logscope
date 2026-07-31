/**
 * @file workspace_store_test.cpp
 * @brief Unit tests for WorkspaceStore (M15.3 / SW1, S2.1).
 */

#include <gtest/gtest.h>

#include <filesystem>

#include "web_config.hpp"
#include "workspace_store.hpp"

namespace
{

scope::web::WebConfig testConfig(const std::filesystem::path& workspaceRoot)
{
    scope::web::WebConfig config = scope::web::WebConfig::defaults();
    config.workspaceDir = scope::foundation::Path(workspaceRoot.string());

    return config;
}

} // namespace

TEST(WorkspaceStoreTest, CreateListGetUpdateDeleteRoundTrip)
{
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "logscope-workspace-store-test";
    std::filesystem::remove_all(root);

    scope::web::WorkspaceStore store(testConfig(root));

    scope::web::WorkspaceCreateRequest createRequest;
    createRequest.name = "incident-alpha";
    createRequest.description = "test workspace";

    const auto created = store.create(createRequest);
    ASSERT_TRUE(created);
    EXPECT_FALSE(created->id.empty());
    EXPECT_EQ("incident-alpha", created->name);

    const auto listed = store.list(100);
    ASSERT_TRUE(listed);
    ASSERT_EQ(1U, listed->workspaces.size());
    EXPECT_EQ(created->id, listed->workspaces.front().id);

    const auto fetched = store.getMetadata(created->id);
    ASSERT_TRUE(fetched);
    EXPECT_EQ("incident-alpha", fetched->name);

    scope::web::WorkspaceUpdateRequest updateRequest;
    updateRequest.name = "incident-beta";
    updateRequest.description = "updated";

    const auto updated = store.updateMetadata(created->id, updateRequest);
    ASSERT_TRUE(updated);
    EXPECT_EQ("incident-beta", updated->name);
    EXPECT_EQ("updated", updated->description);

    const auto removed = store.remove(created->id);
    ASSERT_TRUE(removed);

    const auto missing = store.getMetadata(created->id);
    EXPECT_FALSE(missing);

    std::filesystem::remove_all(root);
}

TEST(WorkspaceStoreTest, RejectsInvalidWorkspaceId)
{
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "logscope-workspace-store-invalid";
    std::filesystem::remove_all(root);

    scope::web::WorkspaceStore store(testConfig(root));

    EXPECT_FALSE(store.getMetadata("../escape"));
    EXPECT_FALSE(store.getMetadata("not-a-uuid"));
    EXPECT_FALSE(scope::web::WorkspaceStore::isValidWorkspaceId(".."));
    EXPECT_FALSE(scope::web::WorkspaceStore::isValidWorkspaceId("550e8400-e29b-41d4-a716-446655440000/extra"));

    std::filesystem::remove_all(root);
}

TEST(WorkspaceStoreTest, SnapshotPathStaysUnderWorkspaceRoot)
{
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "logscope-workspace-store-snapshot";
    std::filesystem::remove_all(root);

    scope::web::WorkspaceStore store(testConfig(root));

    scope::web::WorkspaceCreateRequest createRequest;
    createRequest.name = "snapshot-test";

    const auto created = store.create(createRequest);
    ASSERT_TRUE(created);

    const auto snapshotPath = store.snapshotPathFor(created->id);
    ASSERT_TRUE(snapshotPath);
    EXPECT_NE(std::string::npos, snapshotPath->string().find(created->id));

    std::filesystem::remove_all(root);
}
