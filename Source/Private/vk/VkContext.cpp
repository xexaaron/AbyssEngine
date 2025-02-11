#include <Windows.h>
#include "vk/VkContext.h"
#include "Core/Log.h"
#include <vector>
#include <cstring>

#ifdef _WIN32
    #include <Windows.h>
    #include "vulkan/vulkan_win32.h"
    #define VK_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(__linux__)
    #include "vulkan/vulkan_xlib.h"
    #define GLFW_EXPOSE_NATIVE_X11
    #include <glfw/glfw3native.h>
    #define VK_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_X11_SURFACE_EXTENSION_NAME
#else
    #error "Unsupported Platform"
#endif

namespace aby::vk {

    Context::Context(const AppInfo& info, Ref<Window> window) :
        aby::Context(info, window) 
    {
        std::vector<const char*> instance_extensions = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_PLATFORM_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        };
        //instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        std::vector<const char*> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        std::vector<const char*> validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };
        
        m_Instance.create(info, instance_extensions, validation_layers);
        m_Debugger.create(m_Instance);
        m_Surface.create(m_Instance, window);
        m_Devices.create(m_Instance, m_Surface, device_extensions);

        DBG(
            VkPhysicalDeviceProperties best_device_properties;
            vkGetPhysicalDeviceProperties(m_Devices.physical(), &best_device_properties);
            ABY_DBG("Using Physical Device: {}", best_device_properties.deviceName);
            ABY_DBG("  Type: {}", helper::to_string(best_device_properties.deviceType));
            ABY_DBG("  API Version: {}.{}.{}",
                VK_API_VERSION_MAJOR(best_device_properties.apiVersion),
                VK_API_VERSION_MINOR(best_device_properties.apiVersion),
                VK_API_VERSION_PATCH(best_device_properties.apiVersion)
            );
            ABY_DBG("  Driver Version: {}", best_device_properties.driverVersion);

            ABY_DBG("Enabled Extensions: {}", instance_extensions.size() + device_extensions.size());
            ABY_DBG("Instance Extensions: {}", instance_extensions.size());
            for (std::size_t i = 0; i < instance_extensions.size(); i++) {
                ABY_DBG("({}) -- {}", i + 1, instance_extensions[i]);
            }
            ABY_DBG("Device Extensions: {}", device_extensions.size());
            for (std::size_t i = 0; i < device_extensions.size(); i++) {
                ABY_DBG("({}) -- {}", i + 1 + instance_extensions.size(), device_extensions[i]);
            }
        )
    }

    Ref<Context> Context::create(const AppInfo& app_info, Ref<Window> window) {
        return create_ref<Context>(app_info, window);
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