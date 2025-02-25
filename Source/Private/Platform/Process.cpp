#include "Platform/Process.h"

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    #define POSIX 
#endif

#ifdef _WIN32
    #include "Platform/win32/ProcessWin32.h"
    #define PLATFORM_NAMESPACE win32
#elif defined(POSIX)
    #include "Platform/posix/ProcessPosix.h"
    #define PLATFORM_NAMESPACE posix
#endif

namespace aby::sys {
    
    Unique<Process> Process::create(const std::function<void(const std::string&)>& read_callback) {
        return create_unique<PLATFORM_NAMESPACE::Process>(read_callback);
    }

    Process::Process(const std::function<void(const std::string&)>& read_callback) :
        m_Read(read_callback)
    {

    }


}