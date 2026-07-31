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
