#include "Core/App.h"
#include "Core/Log.h"
#include "Rendering/UI/Widget.h"
#include "vk/VkRenderer.h"
#ifdef _WIN32
    #include <Windows.h>
#else
    #include <unistd.h>
#endif

// Static data
namespace aby {
    
    fs::path get_exe_path() {
    #ifdef _WIN32
        char path_buff[MAX_PATH];
        ABY_ASSERT(GetModuleFileName(NULL, path_buff, MAX_PATH) != 0, "");
        return fs::path(path_buff);
    #else
        char path_buff[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", path_buff, PATH_MAX);
        if (count == -1) {
            perror("readlink");
            return "";
        }
        return fs::path(std::string(path_buff, count));
    #endif
    }

    fs::path App::s_Path = get_exe_path();


    fs::path App::bin() {
        return s_Path.parent_path();
    }

    const fs::path& App::exe() {
        return s_Path;
    }
}

namespace aby {

    App::App(const AppInfo& info, glm::u32vec2 window_size) : 
        m_Window(Window::create(info.bInherit ? info.Name : "Window", window_size.x, window_size.y)),
        m_Ctx(Context::create(this, m_Window)),
        m_Renderer(Renderer::create(m_Ctx)),
        m_Info(info)
       //  m_ResourceThread(m_Ctx.get())
    {
        m_Window->register_event(this, &App::on_event);
    }

    App::~App() {
        m_Renderer->destroy();
        m_Ctx->destroy();
    }

    void App::run() {
        auto last_time = std::chrono::high_resolution_clock::now();
        float delta_time = 0.0f;
        
        for (auto& object : m_Objects) {
            // TODO: Pool serialization context for object->on_deserialize()
            //       and file resource manager for objects of uuids.
            object->on_create(this, false);
        }

        // m_ResourceThread.sync();

        for (auto& object : m_Objects) {
            if (auto p = std::dynamic_pointer_cast<ui::Widget>(object)) {
                p->on_invalidate();
            }
        }


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
           
        }

        for (auto& obj : m_Objects) {
            // obj->on_serialize(Serializer());
            obj->on_destroy(this);
        }
    }
 
    void App::set_name(const std::string& name) {
        m_Info.Name = name;
        if (m_Info.bInherit) {
            m_Window->set_title(name);
        }
    }

    const std::string& App::name() const {
        return m_Info.Name;
    }

    const AppVersion& App::version() const {
        return m_Info.Version;
    }

    Ref<Window> App::window() {
        return m_Window;
    }
    Ref<Window> App::window() const {
        return m_Window;
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
        auto it = std::remove(m_Objects.begin(), m_Objects.end(), obj);
        if (it != m_Objects.end()) {
            (*it)->on_destroy(this);
            m_Objects.erase(it, m_Objects.end());
        }
    }

    void App::on_event(Event& event) {
        for (auto& obj : m_Objects) {
            obj->on_event(this, event);
        }
    }

    // ResourceThread& App::resource_thread() {
    //     return m_ResourceThread;
    // }
    
    const AppInfo& App::info() const {
        return m_Info;
    }


}