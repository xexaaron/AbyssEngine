#include "Platform/SharedLibrary.h"
#include "Platform/Platform.h"

#ifdef _WIN32  // Windows
    #include "Platform/win32/SharedLibraryWin32.h"
    #define PLATFORM_NAMESPACE win32
#elif defined(POSIX)
    #include "Platform/posix/SharedLibraryPosix.h"
    #define PLATFORM_NAMESPACE posix
#else
    #error "Unsupported platform"
#endif

namespace aby::sys {

    Unique<SharedLibrary> SharedLibrary::create(const fs::path& path) {
        return create_unique<PLATFORM_NAMESPACE::SharedLibrary>(path);
    }

    SharedLibrary::SharedLibrary(const fs::path& path) : m_Path(path) {}

    const fs::path& SharedLibrary::path() const {
        return m_Path;
    }

}