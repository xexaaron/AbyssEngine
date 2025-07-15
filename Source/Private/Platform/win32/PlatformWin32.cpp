#include "Platform/win32/PlatformWin32.h"
#include "Core/Log.h"

#ifdef _WIN32

#include <processthreadsapi.h>

#ifdef _MSVC_VER
	#include <io.h> // _isatty, _fileno
#endif

#include <GLFW/glfw3.h>

namespace aby::sys::win32 {
    
    auto to_string(const std::wstring& wstring) -> std::string {
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0) {
            return "";
        }
        std::string str(size_needed - 1, '\0'); // Remove null terminator
        WideCharToMultiByte(CP_UTF8, 0, wstring.c_str(), -1, &str[0], size_needed, nullptr, nullptr);
        return str;
    }

    std::string to_string_ansi(const std::wstring& wstr) {
        if (wstr.empty()) return {};
        int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_ACP, 0, wstr.data(), (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
        return strTo;
    }

    auto to_wstring(const std::string& string) -> std::wstring {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, nullptr, 0);
        if (size_needed <= 0) {
            return L"";
        }
        std::wstring wstring(size_needed - 1, L'\0'); // Remove null terminator
        MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, &wstring[0], size_needed);
        return wstring;
    }

    auto is_terminal(std::FILE* stream) -> bool  {
    #ifdef _MSVC_VER
        return _isattty(_fileno(stream));
    #else
        DWORD mode;
        HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
        if (out == INVALID_HANDLE_VALUE) return false;
        return GetConsoleMode(out, &mode);
    #endif
	}

    auto set_thread_name(std::thread& thread, const std::string& name) -> bool {
        HANDLE handle = reinterpret_cast<HANDLE>(thread.native_handle());
        if (name.size() > 15) { 
            wchar_t* wdesc = nullptr;
            if (SUCCEEDED(GetThreadDescription(handle, &wdesc))) {
                std::string desc = wdesc ? to_string(wdesc) : "<unknown>";
                LocalFree(wdesc);
                if (name.size() > 15) {
                    ABY_ERR("Thread name exceeds 15 characters (legacy limit). Thread: {}, Name: {}", desc, name);
                }
            }
        }
        std::wstring wname = to_wstring(name);
        if (FAILED(SetThreadDescription(handle, wname.c_str()))) {
            return false;
        }
        return true;
    }

    auto get_exec_path() -> fs::path {
        char path_buff[MAX_PATH];
        ABY_ASSERT(GetModuleFileName(NULL, path_buff, MAX_PATH) != 0, "");
        return fs::path(path_buff);
    }

    auto get_pid() -> int {
        return GetCurrentProcessId();
    }

    auto get_args(int& argc, char**& argv) -> void {
        // Get the full command line as wide string
        LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!argvW) {
            argc = 0;
            argv = nullptr;
            return;
        }

        // Allocate argv array of char*
        argv = new char* [argc];

        for (int i = 0; i < argc; ++i) {
            std::string arg = to_string_ansi(argvW[i]);
            // allocate and copy argument string
            argv[i] = new char[arg.size() + 1];
            memcpy(argv[i], arg.c_str(), arg.size() + 1);
        }

        LocalFree(argvW);
    }

    auto free_args(int argc, char** argv) -> void {
        if (!argv) return;
        for (int i = 0; i < argc; ++i) {
            delete[] argv[i];
        }
        delete[] argv;
    }

    auto glfw_to_platform_keycode(int glfw_key) -> int {
        if (glfw_key >= GLFW_KEY_A && glfw_key <= GLFW_KEY_Z)
            return glfw_key; // Same as 'A' to 'Z'

        if (glfw_key >= GLFW_KEY_0 && glfw_key <= GLFW_KEY_9)
            return glfw_key; // Same as '0' to '9'

        if (glfw_key >= GLFW_KEY_KP_0 && glfw_key <= GLFW_KEY_KP_9)
            return VK_NUMPAD0 + (glfw_key - GLFW_KEY_KP_0);

        switch (glfw_key) {
            // Function keys
        case GLFW_KEY_F1:  return VK_F1;
        case GLFW_KEY_F2:  return VK_F2;
        case GLFW_KEY_F3:  return VK_F3;
        case GLFW_KEY_F4:  return VK_F4;
        case GLFW_KEY_F5:  return VK_F5;
        case GLFW_KEY_F6:  return VK_F6;
        case GLFW_KEY_F7:  return VK_F7;
        case GLFW_KEY_F8:  return VK_F8;
        case GLFW_KEY_F9:  return VK_F9;
        case GLFW_KEY_F10: return VK_F10;
        case GLFW_KEY_F11: return VK_F11;
        case GLFW_KEY_F12: return VK_F12;
        case GLFW_KEY_F13: return VK_F13;
        case GLFW_KEY_F14: return VK_F14;
        case GLFW_KEY_F15: return VK_F15;
        case GLFW_KEY_F16: return VK_F16;
        case GLFW_KEY_F17: return VK_F17;
        case GLFW_KEY_F18: return VK_F18;
        case GLFW_KEY_F19: return VK_F19;
        case GLFW_KEY_F20: return VK_F20;
        case GLFW_KEY_F21: return VK_F21;
        case GLFW_KEY_F22: return VK_F22;
        case GLFW_KEY_F23: return VK_F23;
        case GLFW_KEY_F24: return VK_F24;

            // Special characters
        case GLFW_KEY_SPACE:      return VK_SPACE;
        case GLFW_KEY_APOSTROPHE: return VK_OEM_7;  // '
        case GLFW_KEY_COMMA:      return VK_OEM_COMMA;
        case GLFW_KEY_MINUS:      return VK_OEM_MINUS;
        case GLFW_KEY_PERIOD:     return VK_OEM_PERIOD;
        case GLFW_KEY_SLASH:      return VK_OEM_2;  // /
        case GLFW_KEY_SEMICOLON:  return VK_OEM_1;  // ;
        case GLFW_KEY_EQUAL:      return VK_OEM_PLUS;
        case GLFW_KEY_LEFT_BRACKET:  return VK_OEM_4;
        case GLFW_KEY_BACKSLASH:     return VK_OEM_5;
        case GLFW_KEY_RIGHT_BRACKET: return VK_OEM_6;
        case GLFW_KEY_GRAVE_ACCENT:  return VK_OEM_3; // `

            // Navigation and modifiers
        case GLFW_KEY_ESCAPE:    return VK_ESCAPE;
        case GLFW_KEY_ENTER:     return VK_RETURN;
        case GLFW_KEY_TAB:       return VK_TAB;
        case GLFW_KEY_BACKSPACE: return VK_BACK;
        case GLFW_KEY_INSERT:    return VK_INSERT;
        case GLFW_KEY_DELETE:    return VK_DELETE;
        case GLFW_KEY_RIGHT:     return VK_RIGHT;
        case GLFW_KEY_LEFT:      return VK_LEFT;
        case GLFW_KEY_DOWN:      return VK_DOWN;
        case GLFW_KEY_UP:        return VK_UP;
        case GLFW_KEY_PAGE_UP:   return VK_PRIOR;
        case GLFW_KEY_PAGE_DOWN: return VK_NEXT;
        case GLFW_KEY_HOME:      return VK_HOME;
        case GLFW_KEY_END:       return VK_END;

        case GLFW_KEY_CAPS_LOCK:    return VK_CAPITAL;
        case GLFW_KEY_SCROLL_LOCK:  return VK_SCROLL;
        case GLFW_KEY_NUM_LOCK:     return VK_NUMLOCK;
        case GLFW_KEY_PRINT_SCREEN: return VK_SNAPSHOT;
        case GLFW_KEY_PAUSE:        return VK_PAUSE;

        case GLFW_KEY_LEFT_SHIFT:   return VK_LSHIFT;
        case GLFW_KEY_LEFT_CONTROL: return VK_LCONTROL;
        case GLFW_KEY_LEFT_ALT:     return VK_LMENU;
        case GLFW_KEY_LEFT_SUPER:   return VK_LWIN;
        case GLFW_KEY_RIGHT_SHIFT:  return VK_RSHIFT;
        case GLFW_KEY_RIGHT_CONTROL:return VK_RCONTROL;
        case GLFW_KEY_RIGHT_ALT:    return VK_RMENU;
        case GLFW_KEY_RIGHT_SUPER:  return VK_RWIN;
        case GLFW_KEY_MENU:         return VK_APPS;

            // Keypad
        case GLFW_KEY_KP_DECIMAL: return VK_DECIMAL;
        case GLFW_KEY_KP_DIVIDE:  return VK_DIVIDE;
        case GLFW_KEY_KP_MULTIPLY:return VK_MULTIPLY;
        case GLFW_KEY_KP_SUBTRACT:return VK_SUBTRACT;
        case GLFW_KEY_KP_ADD:     return VK_ADD;
        case GLFW_KEY_KP_ENTER:   return VK_RETURN;

        default:
            return 0;  // Unmapped
        }
    }


}

#endif