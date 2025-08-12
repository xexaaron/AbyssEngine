#include "Rendering/Window.h"
#include "Core/Log.h"
#include "Core/App.h"
#include "Rendering/Texture.h"
#include <glfw/glfw3.h>
#include <imgui/imgui.h>

#ifdef _WIN32
   #include "Platform/win32/WindowWin32.h"
#elif defined(__linux__)
    #include "Platform/win32/WindowWin32.h"
#elif defined(__APPLE__)
    #error "Unsupported Platform"
#endif

namespace aby {

    Window::Window(const WindowInfo& info) :
        m_Callbacks{},
        m_Data{
            .title    = info.title,
            .width    = info.size.x,
            .height   = info.size.y,
            .flags    = info.flags,
            .cursor   = ECursor::ARROW,
            .callback = [this](Event& event) -> void {
                for (auto it = m_Callbacks.rbegin(); it != m_Callbacks.rend(); ++it) {
                    (*it)(event);
                }
            }
        },
        m_Window(nullptr)
    {
        ABY_ASSERT(glfwInit(), "Failed to initialize GLFW");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // OpenGL
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        glfwSetErrorCallback([](int code, const char* description) {
            ABY_ERR("[GLFW] ({}): {}", code, description);
        });

        m_Window = glfwCreateWindow(static_cast<int>(info.size.x), static_cast<int>(info.size.y), info.title.c_str(), nullptr, nullptr);
        if (!m_Window) {
            glfwTerminate();
            ABY_ASSERT(Window::m_Window, "Failed to create GLFW window");
        }

        // glfwMakeContextCurrent(m_Window); // OpenGL
        glfwSetWindowUserPointer(m_Window, &m_Data);
    }

    Unique<Window> Window::create(const WindowInfo& info) {
    #ifdef _WIN32
        return create_unique<sys::win32::Window>(info);
    #elif defined(__linux__)
        return create_unique<sys::posix::Window>(info);
    #endif
    }

    Window::~Window() {
        if (m_Window) {
            glfwDestroyWindow(m_Window);
        }
        glfwTerminate();
    }

    bool Window::is_open() const {
        return !glfwWindowShouldClose(m_Window);
    }

    void Window::initialize() {
        setup_callbacks();
        if ((m_Data.flags & EWindowFlags::VSYNC) != EWindowFlags::NONE) {
            set_vsync(true);
        }
        bool maximize = (m_Data.flags & EWindowFlags::MAXIMIZED) != EWindowFlags::NONE;
        bool minimize = (m_Data.flags & EWindowFlags::MINIMIZED) != EWindowFlags::NONE;
        ABY_ASSERT(!(maximize && minimize), "Invalid usage of EWindowFlags");
        if (maximize) {
            set_maximized(true);
        }
        else if (minimize) {
            set_minimized(true);
        }
        int width, height;
        glfwGetWindowSize(m_Window, &width, &height);
        WindowResizeEvent wr_event(width, height, m_Data.width, m_Data.height);
        m_Data.width = width;
        m_Data.height = height;
        m_Data.callback(wr_event);
    }

    void Window::poll_events() {
        glfwPollEvents();
    }

    void Window::swap_buffers() const {
        //glfwSwapBuffers(m_Window); // OpenGL
    }

    void Window::hide() {
        glfwHideWindow(m_Window);
    }

    void Window::show() {
        glfwShowWindow(m_Window);
    }

    void Window::close() {
        glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
    }
    
    float Window::menubar_height() const {
        return ImGui::GetFrameHeight();
    }

    GLFWwindow* Window::glfw() const {
        return m_Window;
    }
   
    u32 Window::width() const {
        return m_Data.width;
    }

    u32 Window::height() const {
        return m_Data.height;
    }

    glm::u32vec2 Window::size() const {
        return { m_Data.width, m_Data.height };
    }

    double Window::scale() const {
        float x, y;
        glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &x, &y);
        return (double)x;
    }

    int Window::refresh_rate() const {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        return mode->refreshRate;
    }

    void Window::set_title(const std::string& title) {
        m_Data.title = title;
        glfwSetWindowTitle(m_Window, m_Data.title.c_str());
    }

    void Window::set_size(u32 w, u32 h) {
        glfwSetWindowSize(m_Window, static_cast<int>(w), static_cast<int>(h));
    }

    void Window::set_position(u32 x, u32 y) {
        glfwSetWindowPos(m_Window, static_cast<int>(x), static_cast<int>(y));
    }

    void Window::set_minimized(bool minimized) {
        if (minimized) {
            glfwIconifyWindow(m_Window);
        }
        else {
            glfwRestoreWindow(m_Window);
        }
    }

    void Window::set_maximized(bool maximized) {
        if (maximized) {
            glfwMaximizeWindow(m_Window);
        }
        else {
            glfwRestoreWindow(m_Window);
        }
    }

    void Window::set_vsync(bool vsync) {
        if (vsync) {
            m_Data.flags |= EWindowFlags::VSYNC;
        }
        else {
            m_Data.flags &= ~EWindowFlags::VSYNC;
        }
        //glfwSwapInterval(static_cast<int>(vsync)); // OpenGL
    }

    bool Window::is_vsync() const {
        return (m_Data.flags & EWindowFlags::VSYNC) != EWindowFlags::NONE;
    }
    
    bool Window::is_minimized() const {
        return (m_Data.flags & EWindowFlags::MINIMIZED) != EWindowFlags::NONE;
    }
 
    bool Window::is_maximized() const {
        return (m_Data.flags & EWindowFlags::MAXIMIZED) != EWindowFlags::NONE;
    }

    bool Window::is_key_pressed(Button::EKey button) const {
        auto state = glfwGetKey(m_Window, button);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }
    
    bool Window::is_mouse_pressed(Button::EMouse button) const {
        auto state = glfwGetMouseButton(m_Window, button);
        return state == GLFW_PRESS;
    }
    
    glm::fvec2 Window::desktop_resolution() const {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        return { static_cast<float>(mode->width), static_cast<float>(mode->height) };
    }
 
    void Window::register_event(const std::function<void(Event&)>& event) {
        m_Callbacks.push_back(event);
    }

    glm::fvec2 Window::mouse_pos() const {
        double x, y;
        glfwGetCursorPos(m_Window, &x, &y);
        return { static_cast<float>(x), static_cast<float>(y) };
    }

    glm::fvec2 Window::dpi() const {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        ABY_ASSERT(monitor, "Failed to get primary monitor");

        float xscale, yscale;
        glfwGetMonitorContentScale(monitor, &xscale, &yscale);

        int widthMM, heightMM;
        glfwGetMonitorPhysicalSize(monitor, &widthMM, &heightMM);

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        ABY_ASSERT(mode, "Failed to get video mode");
        
        return {
            (static_cast<float>(mode->width)  / (static_cast<float>(widthMM) / 25.4f))  * xscale,
            (static_cast<float>(mode->height) / (static_cast<float>(heightMM) / 25.4f)) * yscale
        };

    }

    void Window::set_cursor(ECursor cursor) {
        if (cursor == m_Data.cursor) {
            return;
        }
        
        GLFWcursor* gcursor = nullptr;
        switch (cursor) {
            case ECursor::ARROW:
                gcursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
                break;
            case ECursor::IBEAM:
                gcursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
                break;
            case ECursor::CROSSHAIR:
                gcursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
                break;
            case ECursor::HAND:
                gcursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
                break;
            case ECursor::HRESIZE:
                gcursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
                break;
            case ECursor::VRESIZE:
                gcursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
                break;
            default:
                gcursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
                break;
        }

        if (gcursor) {
            glfwSetCursor(m_Window, gcursor);
            m_Data.cursor = cursor;
        }
    }
    
    void Window::setup_callbacks() {
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* win, int w, int h) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(win));
            WindowResizeEvent event(w, h, data.width, data.height);
            data.width  = w;
            data.height = h;
            data.callback(event);
        });
        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* win) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(win));
            WindowCloseEvent wc_event;
            data.callback(wc_event);
        });
        glfwSetKeyCallback(m_Window, [](GLFWwindow* win, int key, int scancode, int action, int mods) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(win));
            switch (action) {
                case GLFW_PRESS:
                {
                    KeyPressedEvent kp_event(static_cast<Button::EKey>(key), 0, static_cast<Button::EMod>(mods));
                    data.callback(kp_event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent kr_event(static_cast<Button::EKey>(key));
                    data.callback(kr_event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent kr_event(static_cast<Button::EKey>(key), 1, static_cast<Button::EMod>(mods));
                    data.callback(kr_event);
                    break;
                }
                default:
                    std::unreachable();
            }
        });
        glfwSetCharCallback(m_Window, [](GLFWwindow* win, unsigned int keycode) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(win));
            KeyTypedEvent ev(static_cast<Button::EKey>(keycode));
            data.callback(ev);
        });
        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* win, int button, int action, int mods) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(win));
            double xpos, ypos;
            glfwGetCursorPos(win, &xpos, &ypos);
            glm::u32vec2 pos(
                static_cast<u32>(xpos),
                static_cast<u32>(ypos)
            );
            switch (action) {
                case GLFW_PRESS:
                {
                    MousePressedEvent mp_event(static_cast<Button::EMouse>(button), pos);
                    data.callback(mp_event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseReleasedEvent mr_event(static_cast<Button::EMouse>(button), pos);
                    data.callback(mr_event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    MousePressedEvent mr_event(static_cast<Button::EMouse>(button), pos);
                    data.callback(mr_event);
                    break;
                }
                default:
                    std::unreachable();
            }
        });
        glfwSetScrollCallback(m_Window, [](GLFWwindow* win, double xOffset, double yOffset) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(win));
            MouseScrolledEvent ms_event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            data.callback(ms_event);
        });
        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* win, double xPos, double yPos) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(win));
            MouseMovedEvent ms_event(static_cast<float>(xPos), static_cast<float>(yPos));
            data.callback(ms_event);
        });
        glfwSetWindowIconifyCallback(m_Window, [](GLFWwindow* win, int iconified) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(win));
            if (iconified == GLFW_TRUE) {
                data.flags |= EWindowFlags::MINIMIZED;
                data.flags &= ~EWindowFlags::MAXIMIZED;
            }
            else {
                data.flags &= ~EWindowFlags::MAXIMIZED;
                data.flags &= ~EWindowFlags::MINIMIZED;
            }

            // Force size synchronization
            int width, height;
            glfwGetWindowSize(win, &width, &height);
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(win, &fbWidth, &fbHeight);  // Ensure rendering size is updated

            WindowResizeEvent wr_event(width, height, data.width, data.height);
            data.width = width;
            data.height = height;
            data.callback(wr_event);
        });
        glfwSetWindowMaximizeCallback(m_Window, [](GLFWwindow* win, int maximized) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(win));
            if (maximized == GLFW_TRUE) {
                data.flags |=  EWindowFlags::MAXIMIZED;
                data.flags &= ~EWindowFlags::MINIMIZED;
            }
            else {
                data.flags &= ~EWindowFlags::MAXIMIZED;
                data.flags &= ~EWindowFlags::MINIMIZED;
            }
            
            // Force size synchronization
            int width, height;
            glfwGetWindowSize(win, &width, &height);
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(win, &fbWidth, &fbHeight);  // Ensure rendering size is updated

            WindowResizeEvent wr_event(width, height, data.width, data.height);
            data.width = width;
            data.height = height;
            data.callback(wr_event);
        });
    }
}

