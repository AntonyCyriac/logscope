/**
 * @file rest_json_test.cpp
 */

#include <gtest/gtest.h>

#include "foundation/error.hpp"
#include "rest_json.hpp"
#include "workspace.hpp"

using scope::foundation::Error;
using scope::foundation::ErrorCode;

TEST(RestJsonTest, SuccessEnvelopeWrapsData)
{
    const std::string envelope = scope::web::successEnvelope("{\"ok\": true}");

    EXPECT_EQ("{\"data\": {\"ok\": true}}", envelope);
}

TEST(RestJsonTest, ErrorEnvelopeUsesUpperSnakeCode)
{
    const std::string envelope =
        scope::web::errorEnvelopeFromFoundation(Error(ErrorCode::InvalidArgument, "bad input"));

    EXPECT_NE(std::string::npos, envelope.find("\"code\":\"INVALID_ARGUMENT\""));
    EXPECT_NE(std::string::npos, envelope.find("\"message\":\"bad input\""));
}

TEST(RestJsonTest, HttpStatusMapsInvalidArgumentTo400)
{
    EXPECT_EQ(400, scope::web::httpStatusForError(Error(ErrorCode::InvalidArgument, "x")));
    EXPECT_EQ(404, scope::web::httpStatusForError(Error(ErrorCode::FileNotFound, "x")));
}

TEST(RestJsonTest, FormatArtifactRecordIncludesIsEntryAndMetadata)
{
    scope::workspace::ArtifactRecord artifact;
    artifact.id = "artifact-entry";
    artifact.type = "log";
    artifact.name = "app.log";
    artifact.metadata["role"] = "application";

    const std::string json = scope::web::formatArtifactRecord(artifact, "artifact-entry");

    EXPECT_NE(std::string::npos, json.find("\"isEntry\": true"));
    EXPECT_NE(std::string::npos, json.find("\"role\": \"application\""));
}

TEST(RestJsonTest, FormatInvestigationOpenResultIncludesStory2Fields)
{
    scope::workspace::InvestigationSummary summary;
    summary.hasModel = true;
    summary.lineCount = 42U;

    const std::string json = scope::web::formatInvestigationOpenResult(
        "investigation-id", "artifact-id", "log", scope::foundation::Path("/tmp/app.log"), summary, false);

    EXPECT_NE(std::string::npos, json.find("\"artifactId\": \"artifact-id\""));
    EXPECT_NE(std::string::npos, json.find("\"artifactType\": \"log\""));
    EXPECT_NE(std::string::npos, json.find("\"loadedFromSnapshot\": false"));
}

TEST(RestJsonTest, FormatInvestigationTimelineIncludesEventsAndPagination)
{
    scope::workspace::TimelineEvent event;
    event.id = "event-1";
    event.timestamp = "2026-08-01T10:00:00";
    event.artifactId = "artifact-1";
    event.eventType = "log.line";
    event.message = "service started";
    event.source.artifactId = "artifact-1";
    event.source.artifactType = "log";
    event.source.artifactName = "app.log";
    event.source.lineNumber = 1U;

    scope::workspace::TimelineProjectionResult result;
    result.events.push_back(std::move(event));
    result.truncated = true;
    result.totalMatched = 42U;
    result.warnings.push_back("artifact cap reached");

    const std::string json =
        scope::web::formatInvestigationTimeline("investigation-id", result);

    EXPECT_NE(std::string::npos, json.find("\"investigationId\": \"investigation-id\""));
    EXPECT_NE(std::string::npos, json.find("\"eventType\": \"log.line\""));
    EXPECT_NE(std::string::npos, json.find("\"lineNumber\": 1"));
    EXPECT_NE(std::string::npos, json.find("\"truncated\": true"));
    EXPECT_NE(std::string::npos, json.find("\"totalMatched\": 42"));
    EXPECT_NE(std::string::npos, json.find("artifact cap reached"));
}
