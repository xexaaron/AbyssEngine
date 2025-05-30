#include "Platform/vk/VkSurface.h"
#include "Platform/vk/VkAllocator.h"
#include "Core/Log.h"

#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <Windows.h>
#include "vulkan/vulkan_win32.h"
#define VK_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(__linux__)
#include <X11/Xlib.h>
#include "vulkan/vulkan_xlib.h"
#define GLFW_EXPOSE_NATIVE_X11
#include <glfw/glfw3native.h>
#define VK_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#else
#error "Unsupported Platform"
#endif

namespace aby::vk {

    Surface::Surface() :
        m_Surface(VK_NULL_HANDLE),
        m_Format{},
        m_Instance(VK_NULL_HANDLE)
    {
    }

    Surface::Surface(Instance& instance, Window* window) :
        m_Surface(VK_NULL_HANDLE),
        m_Format{},
        m_Instance(VK_NULL_HANDLE)
    {
		create(instance, window);
	}

	void Surface::create(Instance& instance, Window* window) {
        m_Instance = instance;
        ABY_DBG("vk::Surface::create");
    #ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR ci;
        ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        ci.pNext = nullptr;
        ci.flags = 0;
        ci.hwnd = static_cast<HWND>(window->native());
        ci.hinstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(ci.hwnd, GWLP_HINSTANCE));
        VK_CHECK(vkCreateWin32SurfaceKHR(instance, &ci, IAllocator::get(), &m_Surface));
        ABY_DBG("  Platform Win32");
    #elif defined(__linux__)
        VkXlibSurfaceCreateInfoKHR ci;
        ci.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        ci.pNext = NULL;
        ci.flags = 0;
        ci.window = reinterpret_cast<::Window>(window->native());
        ci.dpy = glfwGetX11Display();
        VK_CHECK(vkCreateXlibSurfaceKHR(instance, &ci, IAllocator::get(), &m_Surface));
        ABY_DBG("  Platform Linux");
    #endif
	}

	void Surface::destroy() {
        vkDestroySurfaceKHR(m_Instance, m_Surface, IAllocator::get());
        ABY_DBG("vk::Surface::destroy");
    }

    VkSurfaceFormatKHR Surface::format(DeviceManager& devices) const {
        std::vector<VkSurfaceFormatKHR> available_formats;
        VK_ENUMERATE(available_formats, GetPhysicalDeviceSurfaceFormatsKHR, devices.physical(), m_Surface);
        for (const auto& format : available_formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }
        return available_formats[0];
    }

	Surface::operator VkSurfaceKHR() {
        return m_Surface;
	}

}