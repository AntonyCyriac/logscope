/**
 * @file workspace_paths.hpp
 * @brief Default workspace paths for persisted indexes.
 */

#pragma once

#include "foundation/path.hpp"

namespace scope::storage
{

[[nodiscard]] foundation::Path defaultIndexDirectory() noexcept;

} // namespace scope::storage
