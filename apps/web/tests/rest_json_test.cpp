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
