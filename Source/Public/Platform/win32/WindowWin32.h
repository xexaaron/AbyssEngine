#pragma once
#include "Core/Window.h"
#include <Windows.h>
#include <shobjidl.h>
#include <propvarutil.h>
#include <propkey.h>
#include <shlguid.h>
#include <comdef.h> 

namespace aby::sys::win32 {

    class Window final : public aby::Window {
    public:
        Window(const WindowInfo& info);
        ~Window();

        void* native() const override;
        u32 menubar_height() const override;
    private:
        void restart();
        void set_dark_mode();
        void setup_window_menu();
        void disable_titlebar();
        void set_window_proc();
    private:
        static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    private: // Additional window options
        std::filesystem::path get_exe() const;
        std::filesystem::path get_watchdog(const std::filesystem::path& exe_path) const;
        std::string get_exe_ansi() const;
        std::string get_watchdog_ansi(const std::string& exe_path) const;
        std::string get_filename_only(const std::string& path) const;
        HRESULT create_shell_link(LPCSTR  pszExePath, LPCSTR  pszArguments, LPCSTR  pszTitle, IShellLinkA** ppsl);
        HRESULT add_tasks_to_list(ICustomDestinationList* pcdl);
        void create_jump_list();
        void delete_jump_list();
    private:
        WNDPROC m_OgProc;
    };

}