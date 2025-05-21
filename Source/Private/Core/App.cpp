#include "Core/App.h"
#include "Core/Log.h"
#include "Widget/Widget.h"
#include "vk/VkRenderer.h"
#include "Platform/Platform.h"

// Static data
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

        auto last_time = std::chrono::high_resolution_clock::now();
        float delta_time = 0.0f;
        while (!m_Window->is_open()) {
            auto current_time = std::chrono::high_resolution_clock::now();
            delta_time = std::chrono::duration<float>(current_time - last_time).count();
            last_time = current_time;
            if (!m_Window->is_minimized()) {
                for (auto& obj : m_Objects) {
                    obj->on_tick(this, Time(delta_time));
                }
                m_Window->swap_buffers();
                m_Window->poll_events();
            }
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



#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <cstdlib>

#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)  
#define new DEBUG_NEW  // Override `new` to track allocations

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
int main(int argc, char* argv[]) {
#if _MSC_VER && !defined(NDEBUG) 
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif
    auto args = aby::setup(argc, argv);
    aby::App& app = aby::main(args);
    app.run();
    return 0;
}