/**
 * @file web_config_test.cpp
 */

#include <gtest/gtest.h>

#include "web_config.hpp"

TEST(WebConfigTest, DerivesCorsFromBindHostAndPort)
{
    scope::web::WebConfig config = scope::web::WebConfig::defaults();
    config.bindHost = "192.168.1.50";
    config.bindPort = 9000;
    config.applyDerivedDefaults();

    EXPECT_EQ(1U, config.corsOrigins.size());
    EXPECT_EQ("http://192.168.1.50:9000", config.corsOrigins.front());
}

TEST(WebConfigTest, DerivesHttpsCorsWhenTlsConfigured)
{
    scope::web::WebConfig config = scope::web::WebConfig::defaults();
    config.bindPort = 8443;
    config.tlsCertPath = scope::foundation::Path("cert.pem");
    config.tlsKeyPath = scope::foundation::Path("key.pem");
    config.applyDerivedDefaults();

    EXPECT_TRUE(config.tlsEnabled());
    EXPECT_STREQ("https", config.urlScheme());

    bool hasHttpsLocalhost = false;

    for (const std::string& origin : config.corsOrigins)
    {
        if (origin == "https://localhost:8443")
        {
            hasHttpsLocalhost = true;
        }
    }

    EXPECT_TRUE(hasHttpsLocalhost);
}

TEST(WebConfigTest, SkipsDerivedCorsWhenUserSet)
{
    scope::web::WebConfig config = scope::web::WebConfig::defaults();
    config.corsOriginsUserSet = true;
    config.corsOrigins = {"http://custom.example:1234"};
    config.bindPort = 9000;
    config.applyDerivedDefaults();

    EXPECT_EQ(1U, config.corsOrigins.size());
    EXPECT_EQ("http://custom.example:1234", config.corsOrigins.front());
}

TEST(WebConfigTest, M153Defaults)
{
    const scope::web::WebConfig defaults = scope::web::WebConfig::defaults();

    EXPECT_EQ(100, defaults.workspacesListLimit);
    EXPECT_EQ(10ULL * 1024ULL * 1024ULL, defaults.asyncAnalyzeThresholdBytes);
    EXPECT_EQ(3600, defaults.jobTtlSeconds);
    EXPECT_EQ(1, defaults.jobMaxConcurrentPerSession);
}

TEST(WebConfigTest, ListenUrlReflectsTls)
{
    scope::web::WebConfig config = scope::web::WebConfig::defaults();
    config.bindHost = "127.0.0.1";
    config.bindPort = 8443;
    config.tlsCertPath = scope::foundation::Path("cert.pem");
    config.tlsKeyPath = scope::foundation::Path("key.pem");

    EXPECT_EQ("https://127.0.0.1:8443", config.listenUrl());
}
