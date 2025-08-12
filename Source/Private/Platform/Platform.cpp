#include "Platform/Platform.h"
#include "Core/Log.h"
#include <iostream>

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    #define POSIX 
#endif

#ifdef _WIN32  // Windows
    #include "Platform/win32/PlatformWin32.h"
    #define PLATFORM_NAMESPACE win32
#elif defined(POSIX)
    #include "Platform/posix/PlatformPosix.h"
    #define PLATFORM_NAMESPACE posix
#else
    #error "Unsupported platform"
#endif
#include <cstdio>

namespace aby::sys {
    
    auto is_terminal(const std::ostream& os) -> bool {
        FILE* stream = nullptr;
        if (&os == &std::cout) {
            stream = stdout;
        }
        else if (&os == &std::cerr) {
            stream = stderr;
        }
        else {
            ABY_ERR("std::ostream is not cout or cerr");
        }
        return PLATFORM_NAMESPACE::is_terminal(stream);
    }

    auto set_thread_name(std::thread& thread, const std::string& name) -> bool {
        return PLATFORM_NAMESPACE::set_thread_name(thread, name);
    }

    auto get_exec_path() -> fs::path {
        return PLATFORM_NAMESPACE::get_exec_path();
    }

    auto get_pid() -> int {
        return PLATFORM_NAMESPACE::get_pid();
    }

    auto get_args(int& argc, char**& argv) -> void {
        return PLATFORM_NAMESPACE::get_args(argc, argv);
    }

    auto free_args(int argc, char** argv) -> void {
        return PLATFORM_NAMESPACE::free_args(argc, argv);
    }

    auto glfw_to_platform_keycode(int glfw_key) -> int {
        return PLATFORM_NAMESPACE::glfw_to_platform_keycode(glfw_key);
    }

    auto get_last_err() -> std::string {
        return PLATFORM_NAMESPACE::get_last_err();
    }

	auto open_file_dialog(App* app, const fs::path& start_dir = "", EFileType filter = EFileType::ANY) -> fs::path {
        return PLATFORM_NAMESPACE::open_file_dialog(app, start_dir, static_cast<int>(filter));
    } 


}