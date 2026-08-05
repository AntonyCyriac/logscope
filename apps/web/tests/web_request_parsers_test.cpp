/**
 * @file web_request_parsers_test.cpp
 */

#include <gtest/gtest.h>

#include "foundation/path.hpp"
#include "web_config.hpp"
#include "web_request_parsers.hpp"

TEST(WebRequestParsersTest, RejectsServerPathWhenAllowlistEmpty)
{
    scope::web::WebConfig config = scope::web::WebConfig::defaults();
    config.allowServerPaths = true;
    config.allowedPathRoots.clear();

    const auto result = scope::web::validateServerPath(config, scope::foundation::Path("samples/sample.log"));

    EXPECT_FALSE(result);
    EXPECT_EQ(scope::foundation::ErrorCode::InvalidArgument, result.error().code());
}

TEST(WebRequestParsersTest, AcceptsPathUnderConfiguredRoot)
{
    scope::web::WebConfig config = scope::web::WebConfig::defaults();
    config.allowServerPaths = true;
    config.allowedPathRoots = {scope::foundation::Path("samples")};

    const auto result = scope::web::validateServerPath(config, scope::foundation::Path("samples/sample.log"));

    EXPECT_TRUE(result);
}

TEST(WebRequestParsersTest, ParsesInvestigationCreateRequest)
{
    const scope::web::InvestigationCreateBody request =
        scope::web::parseInvestigationCreateRequest(R"({"name":"incident","description":"outage","captureSession":true})");

    EXPECT_EQ("incident", request.name);
    EXPECT_EQ("outage", request.description);
    EXPECT_TRUE(request.captureSession);
}

TEST(WebRequestParsersTest, ParsesArtifactAddRequestForPstackWithRole)
{
    const auto request = scope::web::parseArtifactAddRequest(
        R"({"type":"pstack","sourcePath":"/tmp/threads.txt","name":"threads","role":"application"})");

    ASSERT_TRUE(request);
    EXPECT_EQ("pstack", request->type);
    EXPECT_EQ("/tmp/threads.txt", request->sourcePath);
    EXPECT_EQ("threads", request->name);
    EXPECT_EQ("application", request->role);
}

TEST(WebRequestParsersTest, ParsesArtifactAddRequestForCore)
{
    const auto request = scope::web::parseArtifactAddRequest(
        R"({"type":"core","sourcePath":"/tmp/dump.core","name":"core.dump"})");

    ASSERT_TRUE(request);
    EXPECT_EQ("core", request->type);
    EXPECT_EQ("/tmp/dump.core", request->sourcePath);
}

TEST(WebRequestParsersTest, RejectsArtifactAddRequestWithoutPstackSourcePath)
{
    const auto request = scope::web::parseArtifactAddRequest(R"({"type":"pstack","name":"threads"})");

    ASSERT_FALSE(request);
    EXPECT_EQ(scope::foundation::ErrorCode::InvalidArgument, request.error().code());
}

TEST(WebRequestParsersTest, ParsesInvestigationOpenRequestWithArtifactId)
{
    const scope::web::InvestigationOpenRequest request = scope::web::parseInvestigationOpenRequest(
        R"({"artifactId":"00000000-0000-4000-8000-000000000099"})");

    EXPECT_EQ("00000000-0000-4000-8000-000000000099", request.artifactId);
}

TEST(WebRequestParsersTest, ParsesInvestigationOpenRequestEmptyBody)
{
    const scope::web::InvestigationOpenRequest request = scope::web::parseInvestigationOpenRequest("{}");

    EXPECT_TRUE(request.artifactId.empty());
}

TEST(WebRequestParsersTest, ParsesInvestigationTimelineQueryParameters)
{
    const scope::web::InvestigationTimelineQuery query =
        scope::web::parseInvestigationTimelineQuery("25", "5", "desc");

    EXPECT_EQ(25U, query.options.limit);
    EXPECT_EQ(5U, query.options.offset);
    EXPECT_EQ(scope::workspace::TimelineSortOrder::Descending, query.options.order);
}
