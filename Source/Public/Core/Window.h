#pragma once
#include "Core/Common.h"
#include "Core/Event.h"
#include <string>
#include <stdexcept>
#include <vector>
#include <glm/glm.hpp>

class GLFWwindow;

namespace aby {

    // Enum operators below
    enum class EWindowFlags {
        NONE      = 0,
        VSYNC     = BIT(0),
        MINIMIZED = BIT(1),
        MAXIMIZED = BIT(2),
    };
    DECLARE_ENUM_OPS(EWindowFlags);

    struct WindowData {
        std::string   title;
        std::uint32_t width;
        std::uint32_t height;
        EWindowFlags  flags;
        std::function<void(Event&)> callback;
    };

    struct WindowInfo {
        glm::u32vec2 size = { 800, 600 };
        EWindowFlags flags = EWindowFlags::NONE;
        std::string  title = "Window"; // if inheriting app name then leave this blank
    };


}

namespace aby {



    class Window {
    public:
        Window(const WindowInfo& info);
        ~Window();

        static Unique<Window> create(const WindowInfo& info);

        /**
        * @brief Initalize the window based on flags provided in ctor.
        *        Allows time to setup callbacks through register_event
        *        If the window starts maximized, that event might
        *        need to be recieved still. Also sends an inital window
        *        resize event regardless of the flags.
        */
        void initalize();
        void poll_events() const;
        void swap_buffers() const;
        void close(); 

        void set_cursor(ECursor cursor);
        void set_title(const std::string& title);
        void set_size(std::uint32_t w, std::uint32_t h);
        void set_position(std::uint32_t x, std::uint32_t y);
        void set_vsync(bool vsync);
        void set_minimized(bool minimized);
        void set_maximized(bool maximized);
        
        /**
        * @return Windows -> ::HWND
        * @return Linux   -> ::Window
        */
        void*         native() const;
        GLFWwindow*   glfw() const;
        double        scale() const;
        std::uint32_t width() const;
        std::uint32_t height() const;
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
    private:
        WindowData m_Data;
        GLFWwindow* m_Window;
        std::vector<std::function<void(Event&)>> m_Callbacks;
    };
}
