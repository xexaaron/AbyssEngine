#include "Core/Log.h"
#include "Core/EntryPoint.h"
#include "Platform/Platform.h"
#include <vector>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#endif

#if _MSC_VER && !defined(NDEBUG) 

#define _CRTDBG_MAP_ALLOC
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <cstdlib>
#endif

#include <cef_command_line.h>
#include <cef_app.h>

#endif

namespace aby {
    std::vector<std::string> setup(int argc, char** argv) {
        App::m_ExePath = argv[0];
        std::vector<std::string> args(argc, "");
        for (int i = 0; i < argc; i++) {
            args[i] = argv[i];
        }
        return args;
    }
    
#ifdef _WIN32
    bool create_console() {
        bool console_attached = AttachConsole(ATTACH_PARENT_PROCESS);
        if (console_attached) {
            ABY_ASSERT(freopen("CONOUT$", "w", stdout) != nullptr);
            ABY_ASSERT(freopen("CONOUT$", "w", stderr) != nullptr);
        }
        else {
            AllocConsole();
            ABY_ASSERT(freopen("CONOUT$", "w", stdout) != nullptr);
            ABY_ASSERT(freopen("CONOUT$", "w", stderr) != nullptr);
        }
        std::cout.clear();
        std::cerr.clear();
        std::cout << "\n" << std::endl;
        return console_attached;
    }

    void destroy_console(bool console_attached) {
#ifndef NDEBUG
        std::cout.flush();
        std::cerr.flush();
        fclose(stdout);
        fclose(stderr);
        if (!console_attached) {
            FreeConsole();
        }
#endif // NDEBUG
    }

    void setup_debug_state() {
#if defined(_MSC_VER) && defined(_CRTDBG_MAP_ALLOC) 
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif // _MSC_VER   &&   _CRTDBG_MAP_ALLOC
    }

    struct ScopedArgs {
        ScopedArgs(HINSTANCE inst) : argc(0), argv(nullptr), cef(inst) { aby::sys::get_args(argc, argv); }
        ~ScopedArgs() { aby::sys::free_args(argc, argv); }

        int argc;
        char** argv;
        CefMainArgs cef;
    };

#endif
}


#ifdef _WIN32

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, int nshowcmd) {
    bool console_attached = aby::create_console();
    aby::setup_debug_state();

    void* sandbox_info = nullptr;
#if defined(CEF_USE_SANDBOX)
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif

    aby::ScopedArgs args(hinstance);
    int ec = CefExecuteProcess(args.cef, nullptr, sandbox_info);
    if (ec >= 0) return ec; // Subprocess finished.
    
    aby::App& app = aby::main(aby::setup(args.argc, args.argv));
    app.run();

    aby::destroy_console(console_attached);
    return 0;
}

#else

int main(int argc, char* argv[]) {
    auto args = aby::setup(argc, argv);
    aby::App& app = aby::main(args);
    app.run();
    return 0;
}

#endif
