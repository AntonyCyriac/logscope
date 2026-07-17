/**
 * @file version.cpp
 * @brief Implements the Version class.
 */

#include "version.hpp"

#include <string>

namespace scope::foundation
{

std::string Version::toString() const
{
    return std::to_string(m_major) + "." + std::to_string(m_minor) + "." + std::to_string(m_patch);
}

} // namespace scope::foundation
