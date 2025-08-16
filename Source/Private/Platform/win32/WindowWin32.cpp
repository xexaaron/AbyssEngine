#ifdef _WIN32
#include "Platform/win32/WindowWin32.h"
#include "Utility/Thread.h"
#include "Core/Log.h" 

#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>
#include <dwmapi.h>
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
    #define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#include <imgui/imgui.h>

#define ABY_COMMAND_RESTART 2

namespace aby::sys::win32 {

    Window::Window(const WindowInfo& info) :
        aby::Window(info),
        m_OgProc(nullptr)
    {
        set_dark_mode();
        setup_window_menu();
        set_window_proc();
        create_jump_list();
        disable_titlebar();
    }

    Window::~Window() {
        delete_jump_list();
    }

    void Window::begin_drag() {
        auto hwnd  = static_cast<HWND>(native());
        auto mouse = mouse_pos();

        ReleaseCapture();
        SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
        POINT pt = { static_cast<LONG>(mouse.x), static_cast<LONG>(mouse.y) };
        ScreenToClient(hwnd, &pt);
        LPARAM lparam = MAKELPARAM(pt.x, pt.y);
        WPARAM wparam = MK_LBUTTON;
        SendMessage(hwnd, WM_LBUTTONDOWN, wparam, lparam);
        SendMessage(hwnd, WM_LBUTTONUP, 0, lparam);
    }

    void Window::become_bg_task() {
        auto hwnd = static_cast<HWND>(native());

        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowLong(hwnd, GWL_EXSTYLE, (exStyle & ~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW);
        SetWindowPos(
            hwnd, nullptr,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
        );

        ShowWindow(hwnd, SW_HIDE);
    }

    void Window::become_fg_task() {
        auto hwnd = static_cast<HWND>(native());

        LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowLong(hwnd, GWL_EXSTYLE, (exStyle & ~WS_EX_TOOLWINDOW) | WS_EX_APPWINDOW);

        SetWindowPos(
            hwnd, nullptr,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED
        );

        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);
    }


    void* Window::native() const {
        return glfwGetWin32Window(m_Window);
    }

    void Window::set_dark_mode() {
        BOOL use_dark_mode = TRUE;
        HWND hwnd = static_cast<HWND>(native());
        DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &use_dark_mode, sizeof(use_dark_mode));
    }

    void Window::setup_window_menu() {
        auto hwnd = static_cast<HWND>(native());
        HMENU sys_menu = GetSystemMenu(hwnd, FALSE);
        AppendMenuA(sys_menu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(sys_menu, MF_STRING, ABY_COMMAND_RESTART, "Restart");
        DrawMenuBar(hwnd);
    }

    void Window::set_window_proc() {
        auto hwnd = static_cast<HWND>(native());
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
        m_OgProc = (WNDPROC)SetWindowLongPtrA(hwnd, GWLP_WNDPROC, (LONG_PTR)&Window::wnd_proc);
    }

    void Window::disable_titlebar() {
        HWND hWnd = glfwGetWin32Window(m_Window);
        LONG_PTR lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
        lStyle |= WS_THICKFRAME;
        lStyle &= ~WS_CAPTION;
        SetWindowLongPtr(hWnd, GWL_STYLE, lStyle);
        RECT windowRect;
        GetWindowRect(hWnd, &windowRect);
        int width = windowRect.right - windowRect.left;
        int height = windowRect.bottom - windowRect.top;
        SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_FRAMECHANGED | SWP_NOMOVE);
    }

    void Window::restart() {
        auto exe_path = get_exe();
        auto watchdog = get_watchdog(exe_path);
        std::stringstream ss;
        ss << "" << watchdog.string() << " "
            << "--id " << exe_path.filename().replace_extension("").string() << " "
            << "--cmd " << exe_path.string() << "";
        util::Thread restart([cmd = ss.str()]() {
            std::system(cmd.c_str());
        });
        restart.set_name("Restart Thread");
        ABY_LOG("System: {}", ss.str());
        restart.detach();
        close();
    }

    LRESULT CALLBACK Window::wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        Window* self = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (msg) {
            case WM_NCCALCSIZE: {
                if (!(wparam == TRUE && lparam != NULL)) break;
                NCCALCSIZE_PARAMS* pParams = reinterpret_cast<NCCALCSIZE_PARAMS*>(lparam);
                if (IsZoomed(hwnd)) {
                    const int pad = 8;
                    pParams->rgrc[0].top += pad - 1;
                    pParams->rgrc[0].right -= pad;
                    pParams->rgrc[0].bottom -= pad;
                    pParams->rgrc[0].left += pad;
                }
                else {
                    pParams->rgrc[0].top += 1;
                    pParams->rgrc[0].right -= 2;
                    pParams->rgrc[0].bottom -= 2;
                    pParams->rgrc[0].left += 2;
                }
                return 0;
                return 0;
            }
            case WM_NCPAINT: {
                return 0;
            }
            case WM_NCHITTEST: {
                const int resize_border = 6;

                POINTS mouse_pos = MAKEPOINTS(lparam);
                POINT client_mouse = { mouse_pos.x, mouse_pos.y };
                ScreenToClient(hwnd, &client_mouse);

                RECT rc;
                GetClientRect(hwnd, &rc);

                const bool left = client_mouse.x <= resize_border;
                const bool right = client_mouse.x >= rc.right - resize_border;
                const bool top = client_mouse.y <= resize_border;
                const bool bottom = client_mouse.y >= rc.bottom - resize_border;

                if (top && left)   return HTTOPLEFT;
                if (top && right)  return HTTOPRIGHT;
                if (bottom && left)  return HTBOTTOMLEFT;
                if (bottom && right) return HTBOTTOMRIGHT;
                if (top)    return HTTOP;
                if (bottom) return HTBOTTOM;
                if (left)   return HTLEFT;
                if (right)  return HTRIGHT;

                return HTCLIENT;
            }
            case WM_NCLBUTTONUP: {

            }
            case WM_SYSCOMMAND: {
                if (LOWORD(wparam) == ABY_COMMAND_RESTART) {
                    self->restart();
                    return 0;
                }
                break;
            }
        }

        if (self && self->m_OgProc) {
            return CallWindowProc(self->m_OgProc, hwnd, msg, wparam, lparam);
        } 
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

}

namespace aby::sys::win32 {

    HRESULT Window::add_tasks_to_list(ICustomDestinationList* pcdl) {
        IObjectCollection* poc;
        HRESULT hr = CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&poc));
        if (SUCCEEDED(hr))
        {
            IShellLinkA* psl;
            auto exe_path = get_exe_ansi();
            auto watchdog = get_watchdog_ansi(exe_path);
            auto exe_name = get_filename_only(exe_path) + ".exe";

            std::stringstream ss;
            ss << "/c taskkill /IM \"" << exe_name << "\" /F && start \"\" \"" << exe_path << "\"";
            std::string args = ss.str();
            hr = create_shell_link("C:\\Windows\\System32\\cmd.exe", args.c_str(), "Restart", &psl);
            if (SUCCEEDED(hr))
            {
                hr = poc->AddObject(psl);
                psl->Release();
            }

            if (SUCCEEDED(hr))
            {
                IObjectArray* poa;
                hr = poc->QueryInterface(IID_PPV_ARGS(&poa));
                if (SUCCEEDED(hr))
                {
                    hr = pcdl->AddUserTasks(poa);
                    poa->Release();
                }
            }
            poc->Release();
        }
        return hr;
    }

    HRESULT Window::create_shell_link(LPCSTR pszExePath, LPCSTR pszArguments, LPCSTR pszTitle, IShellLinkA** ppsl) {
        IShellLinkA* psl;
        HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl);
        if (strstr(pszTitle, "Restart") != nullptr) {
            auto icon = get_exe().parent_path() / "Textures" / "ArrowIcon.ico";
            psl->SetIconLocation(icon.string().c_str(), 0);
        }
        if (SUCCEEDED(hr))
        {
            hr = psl->SetPath(pszExePath);
            if (SUCCEEDED(hr))
            {
                hr = psl->SetArguments(pszArguments);
                if (SUCCEEDED(hr))
                {
                    IPropertyStore* pps;
                    hr = psl->QueryInterface(IID_PPV_ARGS(&pps));
                    if (SUCCEEDED(hr))
                    {
                        // PKEY_Title requires wide string, so convert pszTitle to wide
                        std::wstring wtitle;
                        int len = MultiByteToWideChar(CP_ACP, 0, pszTitle, -1, nullptr, 0);
                        if (len > 0)
                        {
                            wtitle.resize(len);
                            MultiByteToWideChar(CP_ACP, 0, pszTitle, -1, &wtitle[0], len);
                        }

                        PROPVARIANT propvar;
                        hr = InitPropVariantFromString(wtitle.c_str(), &propvar);
                        if (SUCCEEDED(hr))
                        {
                            hr = pps->SetValue(PKEY_Title, propvar);
                            if (SUCCEEDED(hr))
                            {
                                hr = pps->Commit();
                                if (SUCCEEDED(hr))
                                {
                                    hr = psl->QueryInterface(IID_PPV_ARGS(ppsl));
                                }
                            }
                            PropVariantClear(&propvar);
                        }
                        pps->Release();
                    }
                }
            }
            psl->Release();
        }
        return hr;
    }

    void Window::create_jump_list() {
     
        ICustomDestinationList* pcdl;
        HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
        if (SUCCEEDED(hr))
        {
            UINT cMinSlots;
            IObjectArray* poaRemoved;
            hr = pcdl->BeginList(&cMinSlots, IID_PPV_ARGS(&poaRemoved));
            if (SUCCEEDED(hr))
            {
                if (SUCCEEDED(hr))
                {
                    hr = add_tasks_to_list(pcdl);
                    if (SUCCEEDED(hr))
                    {
                        hr = pcdl->CommitList();
                    }
                }
                poaRemoved->Release();
            }
            pcdl->Release();
        }
    }

    void Window::delete_jump_list() {
        ICustomDestinationList* pcdl;
        HRESULT hr = CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pcdl));
        if (SUCCEEDED(hr))
        {
            hr = pcdl->DeleteList(NULL);
            pcdl->Release();
        }
    }

    std::string Window::get_exe_ansi() const {
        char buffer[MAX_PATH];
        DWORD len = GetModuleFileNameA(NULL, buffer, MAX_PATH);
        if (len == 0 || len == MAX_PATH)
            return std::string();
        return std::string(buffer, len);
    }

    std::string Window::get_watchdog_ansi(const std::string& exe_path) const {
        size_t pos = exe_path.find_last_of("\\/");
        std::string folder = (pos == std::string::npos) ? "" : exe_path.substr(0, pos + 1);
        return folder + "watchdog.exe";
    }

    std::string Window::get_filename_only(const std::string& path) const {
        size_t pos = path.find_last_of("\\/");
        std::string filename = (pos == std::string::npos) ? path : path.substr(pos + 1);
        size_t dot_pos = filename.find_last_of('.');
        if (dot_pos != std::string::npos) {
            filename = filename.substr(0, dot_pos);
        }

        return filename;
    }

    std::filesystem::path Window::get_exe() const {
        int argc = 0;
        LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
        if (!argvW || argc < 1) return "";
        int size = WideCharToMultiByte(CP_ACP, 0, argvW[0], -1, nullptr, 0, nullptr, nullptr);
        std::string exe_path_str(size, 0);
        WideCharToMultiByte(CP_ACP, 0, argvW[0], -1, &exe_path_str[0], size, nullptr, nullptr);
        LocalFree(argvW);
        std::filesystem::path exe_path(exe_path_str);
        return exe_path;
    }

    std::filesystem::path Window::get_watchdog(const std::filesystem::path& exe_path) const {
#ifndef NDEBUG
        auto watchdog = exe_path.parent_path() / "watchdog.exe";
#else
        auto watchdog = exe_path.parent_path() / "watchdog.exe";
#endif
        return watchdog;
    }

}

#endif