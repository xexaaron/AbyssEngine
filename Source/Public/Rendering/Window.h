#pragma once
#include "Core/Common.h"
#include "Core/Event.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct GLFWwindow;

namespace aby {

    enum class EWindowFlags {
        NONE      = 0,
        VSYNC     = BIT(0),
        MINIMIZED = BIT(1),
        MAXIMIZED = BIT(2),
    };
    DECLARE_ENUM_OPS(EWindowFlags);

    struct WindowData {
        std::string   title;
        u32 width  = 800;
        u32 height = 600;
        EWindowFlags  flags  = EWindowFlags::NONE;
        ECursor       cursor = ECursor::ARROW;
        std::function<void(Event&)> callback = {};
    };

    struct WindowInfo {
        glm::u32vec2 size = { 800, 600 };
        EWindowFlags flags = EWindowFlags::NONE;
        std::string  title = "Window"; // if inheriting app name then leave this blank
    };


}

namespace aby {
    class App;

    class Window {
    public:
        explicit Window(const WindowInfo& info);
        virtual ~Window();

        static Unique<Window> create(const WindowInfo& info);

        /**
        * Initialize the window based on flags provided in ctor.
        * Allows time to set up callbacks through register_event
        */
        void initialize();
        void poll_events();
        void swap_buffers() const;
        void close(); 
        void hide();
        void show();
        virtual void become_bg_task() = 0;
        virtual void become_fg_task() = 0;
        virtual void begin_drag() = 0;

        void set_cursor(ECursor cursor);
        void set_title(const std::string& title);
        void set_size(u32 w, u32 h);
        void set_position(u32 x, u32 y);
        void set_vsync(bool vsync);
        void set_minimized(bool minimized);
        void set_maximized(bool maximized);
        
        /**
        * @return Windows -> ::HWND
        * @return Linux   -> ::Window
        */
        virtual void* native() const = 0;
        float         menubar_height() const;
        GLFWwindow*   glfw() const;
        double        scale() const;
        u32           width() const;
        u32           height() const;
        glm::u32vec2  size() const;
        glm::fvec2    mouse_pos() const;
        int           refresh_rate() const;
        glm::fvec2    desktop_resolution() const;
        glm::fvec2    dpi() const;

        bool is_vsync() const;
        bool is_minimized() const;
        bool is_maximized() const;
        bool is_key_pressed(Button::EKey button) const;
        bool is_mouse_pressed(Button::EMouse button) const;
        bool is_open() const;

        void register_event(const std::function<void(Event&)>& event);
        template <typename T>
        void register_event(T* obj, void(T::* event)(Event&)) {
            m_Callbacks.push_back([obj, event](Event& e) {
                return (obj->*event)(e);
            });
        }
    private:
        void setup_callbacks();
    protected:
        std::vector<std::function<void(Event&)>> m_Callbacks;
        WindowData  m_Data;
        GLFWwindow* m_Window;
    };
}
