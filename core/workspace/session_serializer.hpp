/**
 * @file session_serializer.hpp
 * @brief Serializes investigation sessions to disk.
 */

#pragma once

#include <string>

#include "foundation/path.hpp"
#include "foundation/result.hpp"
#include "investigation_session.hpp"

namespace scope::workspace
{

/**
 * @brief Reads and writes investigation session files.
 */
class SessionSerializer
{
  public:
    /**
     * @brief Serializes a session to a properties-style text format.
     */
    [[nodiscard]] static std::string serialize(const InvestigationSession& session);

    /**
     * @brief Deserializes a session from text content.
     */
    [[nodiscard]] static foundation::Result<InvestigationSession> deserialize(std::string_view content);
};

} // namespace scope::workspace
