/**
 * @file investigation_store_test.cpp
 * @brief Unit tests for InvestigationStore (v2.3.0 / IC.1).
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include "investigation_store.hpp"
#include "web_config.hpp"

namespace
{

scope::web::WebConfig testConfig(const std::filesystem::path& workspaceRoot)
{
    scope::web::WebConfig config = scope::web::WebConfig::defaults();
    config.workspaceDir = scope::foundation::Path(workspaceRoot.string());

    return config;
}

} // namespace

TEST(InvestigationStoreTest, CreateListGetUpdateDeleteRoundTrip)
{
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "logscope-investigation-store-test";
    std::filesystem::remove_all(root);

    scope::web::InvestigationStore store(testConfig(root));

    const auto created = store.create("incident-alpha", "test investigation");
    ASSERT_TRUE(created);
    EXPECT_FALSE(created->id.empty());
    EXPECT_EQ("incident-alpha", created->name);

    const auto listed = store.list(100);
    ASSERT_TRUE(listed);
    ASSERT_EQ(1U, listed->investigations.size());
    EXPECT_EQ(created->id, listed->investigations.front().id);

    const auto fetched = store.get(created->id);
    ASSERT_TRUE(fetched);
    EXPECT_EQ("incident-alpha", fetched->name);

    scope::web::InvestigationUpdateRequest updateRequest;
    updateRequest.name = "incident-beta";
    updateRequest.description = "updated";

    const auto updated = store.update(created->id, updateRequest);
    ASSERT_TRUE(updated);
    EXPECT_EQ("incident-beta", updated->name);
    EXPECT_EQ("updated", updated->description);

    const auto removed = store.remove(created->id);
    ASSERT_TRUE(removed);

    const auto missing = store.get(created->id);
    EXPECT_FALSE(missing);

    std::filesystem::remove_all(root);
}

TEST(InvestigationStoreTest, AddLogAndNoteArtifacts)
{
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "logscope-investigation-store-artifacts";
    std::filesystem::remove_all(root);

    const std::filesystem::path logSource =
        std::filesystem::temp_directory_path() / "logscope-investigation-source.log";

    {
        std::ofstream stream(logSource);
        stream << "2026-08-04 ERROR service failed\n";
    }

    scope::web::InvestigationStore store(testConfig(root));

    const auto created = store.create("artifact-test", "artifacts");
    ASSERT_TRUE(created);

    const auto logArtifact =
        store.addLogArtifact(created->id, scope::foundation::Path(logSource.string()), "app.log");
    ASSERT_TRUE(logArtifact) << "addLogArtifact failed";
    EXPECT_EQ("log", logArtifact->type);

    const auto afterLog = store.get(created->id);
    ASSERT_TRUE(afterLog);
    ASSERT_EQ(1U, afterLog->artifacts.size());

    const auto noteArtifact = store.addNoteArtifact(created->id, "triage", "Customer reported outage.");
    ASSERT_TRUE(noteArtifact);
    EXPECT_EQ("note", noteArtifact->type);

    const auto manifest = store.get(created->id);
    ASSERT_TRUE(manifest);
    EXPECT_EQ(2U, manifest->artifacts.size());
    EXPECT_EQ(logArtifact->id, manifest->primaryArtifactId);

    const auto entryPath = store.resolveEntryLogPath(created->id);
    ASSERT_TRUE(entryPath);
    EXPECT_TRUE(std::filesystem::exists(entryPath->string()));

    std::filesystem::remove_all(root);
    std::filesystem::remove(logSource);
}

TEST(InvestigationStoreTest, RejectsInvalidInvestigationId)
{
    const std::filesystem::path root =
        std::filesystem::temp_directory_path() / "logscope-investigation-store-invalid";
    std::filesystem::remove_all(root);

    scope::web::InvestigationStore store(testConfig(root));

    EXPECT_FALSE(store.get("../escape"));
    EXPECT_FALSE(store.get("not-a-uuid"));
    EXPECT_FALSE(scope::web::InvestigationStore::isValidInvestigationId(".."));

    std::filesystem::remove_all(root);
}
