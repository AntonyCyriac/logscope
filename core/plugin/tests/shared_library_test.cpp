/**
 * @file shared_library_test.cpp
 */

#include <gtest/gtest.h>

#include "shared_library.hpp"

using scope::foundation::Path;
using scope::plugin::isSharedLibraryFile;
using scope::plugin::sharedLibraryExtension;

TEST(SharedLibraryTest, DetectsPlatformExtension)
{
    const std::string extension = sharedLibraryExtension();

#if defined(_WIN32)
    EXPECT_EQ(".dll", extension);
#elif defined(__APPLE__)
    EXPECT_EQ(".dylib", extension);
#else
    EXPECT_EQ(".so", extension);
#endif
}

TEST(SharedLibraryTest, IdentifiesSharedLibraryFiles)
{
    const Path libraryPath("plugins/sample" + sharedLibraryExtension());

    EXPECT_TRUE(isSharedLibraryFile(libraryPath));
    EXPECT_FALSE(isSharedLibraryFile(Path("plugins/readme.txt")));
}

TEST(SharedLibraryTest, MissingLibraryReturnsError)
{
    const auto loaded = scope::plugin::SharedLibrary::load(Path("missing_plugin" + sharedLibraryExtension()));

    ASSERT_TRUE(loaded.hasError());
}
