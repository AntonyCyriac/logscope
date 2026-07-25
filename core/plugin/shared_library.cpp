/**
 * @file shared_library.cpp
 */

#include "shared_library.hpp"

#include <algorithm>
#include <cctype>

#include "foundation/error.hpp"
#include "foundation/string.hpp"

#if defined(_WIN32)
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif
#    include <windows.h>
#else
#    include <dlfcn.h>
#endif

namespace scope::plugin
{

namespace
{

#if defined(_WIN32)
std::wstring toWidePath(const foundation::Path& path)
{
    const std::string native = path.string();
    return std::wstring(native.begin(), native.end());
}
#endif

} // namespace

SharedLibrary::SharedLibrary(SharedLibrary&& other) noexcept
    : m_path(std::move(other.m_path)), m_handle(other.m_handle)
{
    other.m_handle = nullptr;
}

SharedLibrary& SharedLibrary::operator=(SharedLibrary&& other) noexcept
{
    if (this != &other)
    {
        if (m_handle != nullptr)
        {
#if defined(_WIN32)
            FreeLibrary(static_cast<HMODULE>(m_handle));
#else
            dlclose(m_handle);
#endif
        }

        m_path = std::move(other.m_path);
        m_handle = other.m_handle;
        other.m_handle = nullptr;
    }

    return *this;
}

SharedLibrary::~SharedLibrary()
{
    if (m_handle != nullptr)
    {
#if defined(_WIN32)
        FreeLibrary(static_cast<HMODULE>(m_handle));
#else
        dlclose(m_handle);
#endif
    }
}

SharedLibrary::SharedLibrary(foundation::Path path, void* handle)
    : m_path(std::move(path)), m_handle(handle)
{
}

foundation::Result<SharedLibrary> SharedLibrary::load(const foundation::Path& path)
{
#if defined(_WIN32)
    const std::wstring widePath = toWidePath(path);
    HMODULE handle = LoadLibraryW(widePath.c_str());

    if (handle == nullptr)
    {
        return foundation::Result<SharedLibrary>(foundation::Error(
            foundation::ErrorCode::IOError, "Failed to load library: " + path.string()));
    }

    return foundation::Result<SharedLibrary>(SharedLibrary(path, handle));
#else
    void* handle = dlopen(path.string().c_str(), RTLD_NOW | RTLD_LOCAL);

    if (handle == nullptr)
    {
        const char* errorMessage = dlerror();

        return foundation::Result<SharedLibrary>(foundation::Error(
            foundation::ErrorCode::IOError,
            std::string("Failed to load library: ") + (errorMessage != nullptr ? errorMessage : path.string())));
    }

    return foundation::Result<SharedLibrary>(SharedLibrary(path, handle));
#endif
}

foundation::Result<void*> SharedLibrary::resolveSymbol(const char* symbolName) const
{
    if (m_handle == nullptr)
    {
        return foundation::Result<void*>(foundation::Error(foundation::ErrorCode::InvalidArgument,
                                                           "Library is not loaded."));
    }

#if defined(_WIN32)
    void* symbol = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(m_handle), symbolName));

    if (symbol == nullptr)
    {
        return foundation::Result<void*>(foundation::Error(
            foundation::ErrorCode::InvalidArgument, std::string("Symbol not found: ") + symbolName));
    }

    return foundation::Result<void*>(symbol);
#else
    dlerror();
    void* symbol = dlsym(m_handle, symbolName);

    const char* errorMessage = dlerror();

    if (errorMessage != nullptr)
    {
        return foundation::Result<void*>(foundation::Error(
            foundation::ErrorCode::InvalidArgument,
            std::string("Symbol not found: ") + symbolName + " (" + errorMessage + ")"));
    }

    return foundation::Result<void*>(symbol);
#endif
}

bool SharedLibrary::isLoaded() const noexcept
{
    return m_handle != nullptr;
}

const foundation::Path& SharedLibrary::path() const noexcept
{
    return m_path;
}

std::string sharedLibraryExtension()
{
#if defined(_WIN32)
    return ".dll";
#elif defined(__APPLE__)
    return ".dylib";
#else
    return ".so";
#endif
}

bool isSharedLibraryFile(const foundation::Path& path)
{
    const std::string filename = foundation::toLower(path.filename().string());

    const std::string extension = sharedLibraryExtension();

    if (filename.size() < extension.size())
    {
        return false;
    }

    return filename.compare(filename.size() - extension.size(), extension.size(), extension) == 0;
}

} // namespace scope::plugin
