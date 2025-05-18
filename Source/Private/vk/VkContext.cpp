#include "vk/VkContext.h"
#include "Core/App.h"
#include <vector>

#ifdef _WIN32
    #include <Windows.h>
    #include "vulkan/vulkan_win32.h"
    #define VK_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    #include <glfw/glfw3.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
    #include "vulkan/vulkan_xlib.h"
    #define GLFW_EXPOSE_NATIVE_X11
    #include <glfw/glfw3native.h>
    #define VK_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME
    #include <glfw/glfw3.h>
#else
    #error "Unsupported Platform"
#endif


namespace aby::vk {

    Context::Context(App* app, Window* window) :
        aby::Context(app, window) 
    {
        std::vector<const char*> instance_extensions = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_PLATFORM_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        };
        instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        std::vector<const char*> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        std::vector<const char*> validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };
        
        m_Instance.create(app->info(), instance_extensions, validation_layers);
        m_Debugger.create(m_Instance);
        m_Surface.create(m_Instance, window);
        m_Devices.create(m_Instance, m_Surface, device_extensions);
    }

    Ref<Context> Context::create(App* app, Window* window) {
        return create_ref<Context>(app, window);
    }

    void Context::destroy() {
        m_Shaders.clear();
        m_Textures.clear();
        m_Devices.destroy();
        m_Debugger.destroy();
        m_Surface.destroy();
        m_Instance.destroy();
    }

   

    Instance& Context::inst() {
        return m_Instance;
    }

    Debugger& Context::debugger() {
        return m_Debugger;
    }

    DeviceManager& Context::devices() {
        return m_Devices;
    }

    Surface& Context::surface() {
        return m_Surface;
    }
}