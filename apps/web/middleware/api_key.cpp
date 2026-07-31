/**
 * @file api_key.cpp
 */

#include "api_key.hpp"

#include "rest_json.hpp"

#include <httplib.h>

namespace scope::web
{

bool authorizeApiKey(const std::string& configuredKey, const httplib::Request& request,
                     httplib::Response& response)
{
    if (configuredKey.empty())
    {
        return true;
    }

    const auto iterator = request.headers.find(kApiKeyHeader);

    if (iterator == request.headers.end() || iterator->second != configuredKey)
    {
        response.status = 401;
        response.set_content(errorEnvelope("UNAUTHORIZED", "Missing or invalid API key"), "application/json");

        return false;
    }

    return true;
}

} // namespace scope::web
