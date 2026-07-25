/**
 * @file ai_investigation_assistant_test.cpp
 */

#include <gtest/gtest.h>

#include "ai_investigation_assistant.hpp"

using scope::ai::AiConfig;
using scope::ai::AiInvestigationAssistant;
using scope::ai::kProviderNoOp;

TEST(AiInvestigationAssistantTest, ResolvesNoopProvider)
{
    const AiConfig config;
    const AiInvestigationAssistant assistant(config);

    EXPECT_EQ(assistant.provider().id(), kProviderNoOp);
    EXPECT_FALSE(assistant.config().enabled);
}
