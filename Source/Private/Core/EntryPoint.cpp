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

    void set_console_attached(App* app, bool attached_from_parent) {
        app->bConsoleAttached = attached_from_parent;
    }

    void setup_debug_state() {
#if defined(_MSC_VER) && defined(_CRTDBG_MAP_ALLOC) 
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif // _MSC_VER   &&   _CRTDBG_MAP_ALLOC
    }

    struct ScopedArgs {
        ScopedArgs(HINSTANCE inst) : argc(0), argv(nullptr) { aby::sys::get_args(argc, argv); }
        ~ScopedArgs() { aby::sys::free_args(argc, argv); }

        int argc;
        char** argv;
    };

#endif
}


#ifdef _WIN32

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, int nshowcmd) {
    bool console_attached = aby::create_console();
    aby::setup_debug_state();
    { // Ensure app is destroyed before the console is freed.
        aby::ScopedArgs args(hinstance);
        aby::App& app = aby::main(aby::setup(args.argc, args.argv));
        aby::set_console_attached(&app, console_attached);
        app.run();
    }
    // Console destroyed in app destructor due to static variable dtor issues with app.
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
