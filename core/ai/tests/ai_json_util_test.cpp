/**
 * @file ai_json_util_test.cpp
 */

#include <gtest/gtest.h>

#include "ai_json_util.hpp"

using scope::ai::buildChatCompletionRequest;
using scope::ai::escapeJsonString;
using scope::ai::extractChatCompletionContent;

TEST(AiJsonUtilTest, EscapesControlCharacters)
{
    EXPECT_EQ(R"(line\"one\n)", escapeJsonString("line\"one\n"));
}

TEST(AiJsonUtilTest, BuildsChatCompletionRequest)
{
    const std::string request = buildChatCompletionRequest("llama3", "system", "user query");

    EXPECT_NE(std::string::npos, request.find("\"model\":\"llama3\""));
    EXPECT_NE(std::string::npos, request.find("\"role\":\"system\""));
    EXPECT_NE(std::string::npos, request.find("\"content\":\"user query\""));
}

TEST(AiJsonUtilTest, ExtractsAssistantContent)
{
    const std::string response =
        R"({"choices":[{"message":{"role":"assistant","content":"level == ERROR"}}]})";

    const auto content = extractChatCompletionContent(response);

    ASSERT_TRUE(content);
    EXPECT_EQ("level == ERROR", *content);
}
