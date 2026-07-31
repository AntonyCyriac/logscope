/**
 * @file rest_json_test.cpp
 */

#include <gtest/gtest.h>

#include "foundation/error.hpp"
#include "rest_json.hpp"

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
