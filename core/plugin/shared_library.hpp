/**
 * @file shared_library.hpp
 * @brief RAII wrapper for dynamic library loading.
 */

#pragma once

#include <string>

#include "foundation/path.hpp"
#include "foundation/result.hpp"

namespace scope::plugin
{

class SharedLibrary
{
  public:
    SharedLibrary() = default;

    SharedLibrary(const SharedLibrary&) = delete;
    SharedLibrary& operator=(const SharedLibrary&) = delete;

    SharedLibrary(SharedLibrary&& other) noexcept;
    SharedLibrary& operator=(SharedLibrary&& other) noexcept;

    ~SharedLibrary();

    [[nodiscard]] static foundation::Result<SharedLibrary> load(const foundation::Path& path);

    [[nodiscard]] foundation::Result<void*> resolveSymbol(const char* symbolName) const;

    [[nodiscard]] bool isLoaded() const noexcept;

    [[nodiscard]] const foundation::Path& path() const noexcept;

  private:
    explicit SharedLibrary(foundation::Path path, void* handle);

    foundation::Path m_path;
    void* m_handle = nullptr;
};

[[nodiscard]] std::string sharedLibraryExtension();

[[nodiscard]] bool isSharedLibraryFile(const foundation::Path& path);

} // namespace scope::plugin
