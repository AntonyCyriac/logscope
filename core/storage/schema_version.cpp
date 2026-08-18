/**
 * @file schema_version.cpp
 */

#include "schema_version.hpp"

#include <string_view>

namespace scope::storage
{

bool requiresSchemaRebuild(const foundation::Error& error) noexcept
{
    if (error.code() != foundation::ErrorCode::InvalidArgument)
    {
        return false;
    }

    return error.message().find("requires rebuild from source") != std::string::npos;
}

bool isUnsupportedSchemaVersion(const foundation::Error& error) noexcept
{
    if (error.code() != foundation::ErrorCode::InvalidArgument)
    {
        return false;
    }

    return error.message().find("Unsupported index schema version") != std::string::npos;
}

bool requiresCompressionRebuild(const foundation::Error& error) noexcept
{
    if (error.code() != foundation::ErrorCode::InvalidArgument)
    {
        return false;
    }

    return error.message().find("compression settings require rebuild from source") != std::string::npos;
}

} // namespace scope::storage
