#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
    #define POSIX 
#endif

#ifdef POSIX

#include "Platform/posix/WindowPosix.h"
#include <glfw/glfw3.h>

#define GLFW_EXPOSE_NATIVE_X11
#include <glfw/glfw3native.h>



namespace aby::sys::posix {

     Window::Window(const WindowInfo& info) :
        aby::Window(info)
     {
    
    }

    Window::~Window() {

    }

    void* Window::native() const {
        return reinterpret_cast<void*>(glfwGetX11Window(m_Window));
    }

}

#endif