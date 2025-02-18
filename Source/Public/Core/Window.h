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
        NONE = 0,
        VSYNC = (1 << 0),
        MINIMIZED = (1 << 1),
        MAXIMIZED = (1 << 2),
    };

    inline constexpr EWindowFlags operator|(EWindowFlags lhs, EWindowFlags rhs) {
        using T = std::underlying_type_t<EWindowFlags>;
        return static_cast<EWindowFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
    }

    inline constexpr EWindowFlags operator&(EWindowFlags lhs, EWindowFlags rhs) {
        using T = std::underlying_type_t<EWindowFlags>;
        return static_cast<EWindowFlags>(static_cast<T>(lhs) & static_cast<T>(rhs));
    }

    inline constexpr EWindowFlags operator^(EWindowFlags lhs, EWindowFlags rhs) {
        using T = std::underlying_type_t<EWindowFlags>;
        return static_cast<EWindowFlags>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
    }

    inline constexpr EWindowFlags operator~(EWindowFlags flag) {
        using T = std::underlying_type_t<EWindowFlags>;
        return static_cast<EWindowFlags>(~static_cast<T>(flag));
    }

    // Assignment operators
    inline constexpr EWindowFlags& operator|=(EWindowFlags& lhs, EWindowFlags rhs) {
        lhs = lhs | rhs;
        return lhs;
    }

    inline constexpr EWindowFlags& operator&=(EWindowFlags& lhs, EWindowFlags rhs) {
        lhs = lhs & rhs;
        return lhs;
    }

    inline constexpr EWindowFlags& operator^=(EWindowFlags& lhs, EWindowFlags rhs) {
        lhs = lhs ^ rhs;
        return lhs;
    }

    struct WindowData {
        std::string   Title;
        std::uint32_t Width;
        std::uint32_t Height;
        EWindowFlags  Flags;
        std::function<void(Event&)> Callback;
    };

}

namespace aby {

    class Window {
    public:
        Window(const std::string& title, std::uint32_t width, std::uint32_t height);
        ~Window();

        static Ref<Window> create(const std::string& title, std::uint32_t width, std::uint32_t height);

        void poll_events() const;
        void swap_buffers() const;

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
