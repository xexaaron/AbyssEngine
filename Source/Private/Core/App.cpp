#include "Core/App.h"
#include "Core/Log.h"
#include "Widget/Widget.h"
#include "Platform/vk/VkRenderer.h"
#include "Platform/Platform.h"

namespace aby {
    
    fs::path App::m_ExePath = "";

    fs::path App::bin() {
        return m_ExePath.parent_path();
    }

    const fs::path& App::exe() {
        return m_ExePath;
    }

    fs::path App::cache() {
        return bin() / "Cache";
    }
}

namespace aby {

    App::App(const AppInfo& app_info, const WindowInfo& window_info) :
        m_Info(app_info),
        m_Window(Window::create(WindowInfo{
            .size = window_info.size,
            .flags = window_info.flags,
            .title = std::string(app_info.binherit ? app_info.name : window_info.title)
        })),
        m_Ctx(Context::create(this, m_Window.get())),
        m_Renderer(Renderer::create(m_Ctx))
    {
        m_Window->register_event(this, &App::on_event);
        fs::path object_cache = cache() / "Objects";
        if (!fs::exists(object_cache)) {
            fs::create_directories(object_cache);
        }
    }

    App::~App() {
        m_Renderer->destroy();
        m_Ctx->destroy();
    }

    void App::run() {
        m_Ctx->load_thread().sync();

        auto object_cache = cache() / "Objects";
        for (auto& obj : m_Objects) {
            obj->on_create(this, false);
        }

        m_Window->initialize();
        m_Ctx->imgui_init();

        auto last_time = std::chrono::high_resolution_clock::now();
        float delta_time = 0.0f;
        while (m_Window->is_open()) {
            auto current_time = std::chrono::high_resolution_clock::now();
            delta_time = std::chrono::duration<float>(current_time - last_time).count();
            last_time = current_time;

            if (m_Window->is_minimized()) continue;
            
            m_Window->poll_events();
            m_Renderer->on_begin();
            m_Ctx->imgui_new_frame();

            for (auto& obj : m_Objects) {
                obj->on_tick(this, Time(delta_time));
            }

            m_Window->swap_buffers();
            m_Ctx->imgui_end_frame();
            m_Renderer->on_end();

            Logger::flush();
        }

        for (auto& obj : m_Objects) {
            obj->on_destroy(this);
        }
    }
 
    void App::set_name(const std::string& name) {
        m_Info.name = name;
        if (m_Info.binherit) {
            m_Window->set_title(name);
        }
    }

    const std::string& App::name() const {
        return m_Info.name;
    }

    const AppVersion& App::version() const {
        return m_Info.version;
    }

    Window* App::window() {
        return m_Window.get();
    }
    Window* App::window() const {
        return m_Window.get();
    }

    Context& App::ctx() {
        return *m_Ctx;
    }
    const Context& App::ctx() const {
        return *m_Ctx;
    }

    Renderer& App::renderer() {
        return *m_Renderer;
    }

    const Renderer& App::renderer() const {
        return *m_Renderer;
    }

    std::span<Ref<Object>> App::objects() {
        return std::span(m_Objects.begin(), m_Objects.size());
    }

    std::span<const Ref<Object>> App::objects() const {
        return std::span(m_Objects.cbegin(), m_Objects.size());
    }

    void App::add_object(Ref<Object> obj) {
        m_Objects.push_back(obj);
    }

    void App::remove_object(Ref<Object> obj) {
        auto it = std::ranges::remove(m_Objects, obj).begin();
        if (it != m_Objects.end()) {
            (*it)->on_destroy(this);
            m_Objects.erase(it, m_Objects.end());
        }
    }
    void App::on_event(Event& event) {
    #if 0
        ABY_LOG("{}", event.to_string());
    #endif
        for (auto& obj : m_Objects) {
            obj->on_event(this, event);
        }
    }

    const AppInfo& App::info() const {
        return m_Info;
    }
 
    void App::quit()  {
        m_Window->close();
    }
    
    void App::restart() {
    #ifdef _WIN32
        std::string ext = ".exe";
    #else
        std::string ext = "";
    #endif
    #ifndef NDEBUG
        auto watchdog = App::bin() / ("../tools/watchdog/Debug/watchdog" + ext);
    #else
        auto watchdog = App::bin() / ("../tools/watchdog/Release/watchdog" + ext);
    #endif
        std::stringstream ss;
        ss << watchdog.string() << " "
           << App::exe().filename().replace_extension("").string() << " " // monitor this app
           << App::exe().string();                                        // run this app
        Thread restart([cmd = ss.str()]() {
            std::system(cmd.c_str());
        });
        restart.set_name("Restart Thread");
        restart.detach();
        this->quit();
    }

}



#if _MSC_VER && !defined(NDEBUG) 

#define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
    #include <cstdlib>

    #define DEBUG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
    #define new DEBUG_NEW // Override `new` to track allocations
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
   

}



#ifdef _WIN32
    #include <Windows.h>

    std::string wide_to_ansi(const std::wstring& wstr) {
        if (wstr.empty()) return {};
        int size_needed = WideCharToMultiByte(CP_ACP, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_ACP, 0, wstr.data(), (int)wstr.size(), &strTo[0], size_needed, nullptr, nullptr);
        return strTo;
    }

    void get_args(int& argc, char**& argv) {
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
            std::string arg = wide_to_ansi(argvW[i]);
            // allocate and copy argument string
            argv[i] = new char[arg.size() + 1];
            memcpy(argv[i], arg.c_str(), arg.size() + 1);
        }

        LocalFree(argvW);
    }

    void free_args(int argc, char** argv) {
        if (!argv) return;
        for (int i = 0; i < argc; ++i) {
            delete[] argv[i];
        }
        delete[] argv;
    }

    int WINAPI WinMain(_In_ HINSTANCE hinstance, _In_opt_ HINSTANCE hprevinstance, _In_ LPSTR lpcmdline, _In_ int nshowcmd) {
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
    #if _MSC_VER && !defined(NDEBUG) 
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    #endif

        int argc = 0;
        char** argv = nullptr;
        get_args(argc, argv);

        auto args = aby::setup(argc, argv);
        aby::App& app = aby::main(args);
        app.run();

        free_args(argc, argv);
        std::cout.flush();
        std::cerr.flush();
        fclose(stdout);
        fclose(stderr);
        if (!console_attached) {
            FreeConsole();
        }
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

