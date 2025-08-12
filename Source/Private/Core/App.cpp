#include "Core/App.h"
#include "Core/Log.h"

#include "Platform/vk/VkRenderer.h"
#include "Platform/Platform.h"
#include "Rendering/Dockspace.h"
#include "Utility/Profiler.h"

#ifdef _WIN32
    #include <Windows.h>
#endif

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
        util::Profiler::get().set_app(this);
        m_Window->register_event(this, &App::on_event);
        fs::path object_cache = cache() / "Objects";
        if (!fs::exists(object_cache)) {
            fs::create_directories(object_cache);
        }
        LogCfg log_cfg = {};
        Logger::set_cfg(log_cfg);
        m_Dockspace = create_ref<Dockspace>();
        add_object(m_Dockspace);
    }

    App::~App() {
        m_Renderer->destroy();
        m_Ctx->destroy();
#ifdef _WIN32
        std::cout.flush();
        std::cerr.flush();
        fclose(stdout);
        fclose(stderr);
        if (!bConsoleAttached) {
            FreeConsole();
        }
#endif
    }

    void App::run() {
        {
            PROFILE_SCOPE("Initialization");
            m_Ctx->load_thread().sync();
            
            auto object_cache = cache() / "Objects";
            for (auto& obj : m_Objects) {
                obj->on_create(this, false);
            }

            for (auto& plugin : m_Plugins) {
                plugin.plugin->on_load(this);
            }

            m_Window->initialize();
            m_Ctx->imgui_init();
        }
        auto last_time   = std::chrono::high_resolution_clock::now();
        float delta_time = 0.0f;
        while (m_Window->is_open()) {
            PROFILE_SCOPE("Frame Loop");
            auto current_time = std::chrono::high_resolution_clock::now();
            delta_time        = std::chrono::duration<float>(current_time - last_time).count();
            last_time         = current_time;
            m_LastDeltatime   = delta_time;

            if (m_Window->is_minimized()) continue;
            
            m_Window->poll_events();
            m_Renderer->on_begin();
            m_Ctx->imgui_new_frame();

            for (auto& obj : m_Objects) {
                obj->on_tick(this, delta_time);
            }

            for (auto& plugin : m_Plugins) {
                plugin.plugin->on_tick(this, delta_time);
            }

            m_Window->swap_buffers();
            m_Ctx->imgui_end_frame();
            m_Renderer->on_end();

            for (auto& [handle, tex] : m_Ctx->textures())
                if (tex->dirty())
                    tex->sync();

            Logger::flush();

        }
        {
            PROFILE_SCOPE("Deinitialization");
            // Become a "background task" so we can clean up resources but look like we exited fast.
            m_Window->become_bg_task();

            for (auto& obj : m_Objects) {
                obj->on_destroy(this);
            }

            for (auto& plugin : m_Plugins) {
                plugin.plugin->on_unload(this);
            }
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

    void App::register_plugin(const fs::path& plugin) {
        auto plug = Plugin::load(plugin);
        if (!plug) return;
        m_Plugins.push_back(std::move(plug));
    }

    void App::remove_plugin(const std::string& plugin_name) {
        auto it = std::remove_if(m_Plugins.begin(), m_Plugins.end(),
            [&](const LoadedPlugin& plugin) {
                return plugin.plugin->name() == plugin_name;
            }
        );
        m_Plugins.erase(it, m_Plugins.end());
    }

    void App::register_event(const std::function<void(Event&)>& event) {
        m_Window->register_event(event);
    }


    void App::on_event(Event& event) {
    #if 0
        ABY_LOG("{}", event.to_string());
    #endif
        for (auto& obj : m_Objects) {
            obj->on_event(this, event);
        }
        for (auto& plugin : m_Plugins) {
            plugin.plugin->on_event(this, event);
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
        util::Thread restart([cmd = ss.str()]() {
            std::system(cmd.c_str());
        });
        restart.set_name("Restart Thread");
        restart.detach();
        this->quit();
    }

    Dockspace& App::dockspace() {
        return m_Dockspace;
    }
    
    const Dockspace& App::dockspace() const {
        return m_Dockspace;
    }

}



