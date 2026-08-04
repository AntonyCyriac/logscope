/**
 * @file api_key.cpp
 */

#include "api_key.hpp"
#include "api_key_credential.hpp"

#include "rest_json.hpp"

#include <httplib.h>

namespace scope::web
{

bool authorizeApiKey(const ApiKeyCredential& credential, const httplib::Request& request,
                     httplib::Response& response)
{
    if (credential.empty())
    {
        return true;
    }

    const auto iterator = request.headers.find(kApiKeyHeader);
    const std::string presented = iterator == request.headers.end() ? std::string{} : iterator->second;

    if (!credential.verify(presented))
    {
        response.status = 401;
        response.set_content(errorEnvelope("UNAUTHORIZED", "Missing or invalid API key"), "application/json");

        return false;
    }

    return true;
}

} // namespace scope::web
