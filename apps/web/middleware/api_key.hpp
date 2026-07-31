/**
 * @file api_key.hpp
 * @brief Optional API key middleware for logscope-web (M15.1).
 */

#pragma once

#include <string>

namespace httplib
{
class Request;
class Response;
} // namespace httplib

namespace scope::web
{

constexpr const char* kSessionHeader = "X-LogScope-Session";
constexpr const char* kApiKeyHeader = "X-LogScope-Api-Key";

/**
 * @brief Validates the API key when configured.
 *
 * @return true when the request may proceed.
 */
[[nodiscard]] bool authorizeApiKey(const std::string& configuredKey, const httplib::Request& request,
                                 httplib::Response& response);

} // namespace scope::web
