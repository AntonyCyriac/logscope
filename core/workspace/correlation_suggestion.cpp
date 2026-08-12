/**
 * @file correlation_suggestion.cpp
 */

#include "correlation_suggestion.hpp"

namespace scope::workspace
{

std::string correlationKeyToString(const CorrelationKey key)
{
    switch (key)
    {
    case CorrelationKey::RequestId:
        return "request_id";
    case CorrelationKey::TraceId:
        return "trace_id";
    case CorrelationKey::SessionId:
        return "session_id";
    case CorrelationKey::TransactionId:
        return "transaction_id";
    case CorrelationKey::CorrelationId:
        return "correlation_id";
    }

    return "request_id";
}

std::optional<CorrelationKey> parseCorrelationKey(const std::string& value)
{
    if (value == "request_id")
    {
        return CorrelationKey::RequestId;
    }

    if (value == "trace_id")
    {
        return CorrelationKey::TraceId;
    }

    if (value == "session_id")
    {
        return CorrelationKey::SessionId;
    }

    if (value == "transaction_id")
    {
        return CorrelationKey::TransactionId;
    }

    if (value == "correlation_id")
    {
        return CorrelationKey::CorrelationId;
    }

    return std::nullopt;
}

} // namespace scope::workspace
