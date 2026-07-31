/**
 * @file session_store_test.cpp
 */

#include <gtest/gtest.h>

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
