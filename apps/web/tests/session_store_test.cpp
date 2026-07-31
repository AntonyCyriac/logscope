/**
 * @file session_store_test.cpp
 */

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#include "session_store.hpp"

TEST(SessionStoreTest, CreateAndFindWorkspace)
{
    scope::web::SessionStore store;
    const std::string sessionId = store.createWorkspace();

    EXPECT_FALSE(sessionId.empty());
    EXPECT_EQ(1U, store.sessionCount());
    EXPECT_NE(nullptr, store.findSession(sessionId));
}

TEST(SessionStoreTest, ResolveSessionAutoCreates)
{
    scope::web::SessionStore store;
    const std::string sessionId = store.resolveSession("", true);

    EXPECT_FALSE(sessionId.empty());
    EXPECT_EQ(1U, store.sessionCount());
}

TEST(SessionStoreTest, ResolveSessionReturnsEmptyWhenDisabled)
{
    scope::web::SessionStore store;

    EXPECT_TRUE(store.resolveSession("missing", false).empty());
}

TEST(SessionStoreTest, StaleSessionIdDoesNotAutoCreate)
{
    scope::web::SessionStore store;

    EXPECT_TRUE(store.resolveSession("stale-session-id", true).empty());
    EXPECT_EQ(0U, store.sessionCount());
}

TEST(SessionStoreTest, EvictIdleSessionsRemovesExpiredWorkspace)
{
    scope::web::SessionStore store;
    const std::string sessionId = store.createWorkspace();

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    const std::size_t evicted = store.evictIdleSessions(std::chrono::seconds(1));

    EXPECT_EQ(1U, evicted);
    EXPECT_EQ(0U, store.sessionCount());
    EXPECT_EQ(nullptr, store.findSession(sessionId));
}

TEST(SessionStoreTest, TouchSessionRefreshesIdleDeadline)
{
    scope::web::SessionStore store;
    const std::string sessionId = store.createWorkspace();

    std::this_thread::sleep_for(std::chrono::milliseconds(700));
    store.touchSession(sessionId);
    std::this_thread::sleep_for(std::chrono::milliseconds(700));

    const std::size_t evicted = store.evictIdleSessions(std::chrono::seconds(1));

    EXPECT_EQ(0U, evicted);
    EXPECT_EQ(1U, store.sessionCount());
}

TEST(SessionStoreTest, EvictSessionsForCapacityRemovesOldestIdle)
{
    scope::web::SessionStore store;
    const std::string first = store.createWorkspace();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const std::string second = store.createWorkspace();

    const std::size_t evicted = store.evictSessionsForCapacity(1U);

    EXPECT_EQ(1U, evicted);
    EXPECT_EQ(1U, store.sessionCount());
    EXPECT_EQ(nullptr, store.findSession(first));
    EXPECT_NE(nullptr, store.findSession(second));
}

TEST(SessionStoreTest, RemoveSessionCleansTempUpload)
{
    scope::web::SessionStore store;
    const std::string sessionId = store.createWorkspace();
    scope::web::WorkspaceSession* workspace = store.findSession(sessionId);
    ASSERT_NE(nullptr, workspace);

    const std::filesystem::path tempPath =
        std::filesystem::temp_directory_path() / "logscope-session-evict-test.log";
    {
        std::ofstream stream(tempPath, std::ios::binary);
        stream << "data\n";
    }

    workspace->tempUploadPath = tempPath.string();
    EXPECT_TRUE(store.removeSession(sessionId));
    EXPECT_FALSE(std::filesystem::exists(tempPath));
}
