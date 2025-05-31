#pragma once
#include "Core/Common.h"
#include <vulkan/vulkan.h>
#include <iostream>
#include <cstdint>
#include <vector>

#define VK_CHECK(x) do {                                                          \
    VkResult result = (x);                                                        \
    if (result != VK_SUCCESS) {                                                   \
        aby::Logger::Assert("[File:{}:{}] {}",                                     \
            std::string_view(__FILE__).substr(                                    \
                std::string_view(__FILE__).find_last_of("/\\") + 1),              \
            __LINE__,                                                             \
            ABY_FUNC_SIG                                                          \
        );                                                                        \
        ::aby::Logger::Assert("{}", ::aby::vk::helper::to_string(result));        \
        ::aby::Logger::flush();                                                   \
        ABY_DBG_BREAK();                                                          \
    }                                                                             \
} while(0)

#define VK_CHECK_HANDLE(x) do {                                                                     \
        if (x == VK_NULL_HANDLE) {                                                                  \
            aby::Logger::Assert("File:{}:{}\n{}",                                                   \
                std::string_view(__FILE__).substr(                                                  \
                    std::string_view(__FILE__).find_last_of("/\\") + 1),                            \
                __LINE__,                                                                           \
                ABY_FUNC_SIG                                                                        \
            );                                                                                      \
            ::aby::Logger::Assert("!(({}){} != VK_NULL_HANDLE)", typeid(decltype(x)).name(), #x);   \
            ::aby::Logger::Assert("Handle is null");                                                \
        }                                                                                           \
    } while(0)

#define VK_LOAD_INST_PROC(inst, function) do {                                                  \
        function = (PFN_##function)vkGetInstanceProcAddr(inst, #function);                      \
        if (function == NULL) { ABY_ERR("Failed to load instance function {}", #function); return {}; } \
    } while (0)

#define VK_LOAD_DEV_PROC(inst, function) do {                                                   \
        function = (PFN_##function)vkGetDeviceProcAddr(inst, #function);                        \
        if (function == NULL) { ABY_ERR("Failed to load device function {}", #function); return {}; }   \
    } while (0)

#define VK_DEF_PROC(x) PFN_##x x;

#define VK_ENUMERATE(out, enumerator_fn, ...) do {                      \
        uint32_t enumerator_fn##_count = 0;                             \
        enumerator_fn(__VA_ARGS__, &enumerator_fn##_count, nullptr);    \
        out.resize(enumerator_fn##_count);                              \
        enumerator_fn(__VA_ARGS__, &enumerator_fn##_count, out.data()); \
    } while(0)

namespace aby::vk {

    constexpr static std::size_t   MAX_FRAMES_IN_FLIGHT     = 2;
    constexpr static u32 MAX_BINDLESS_RESOURCES   = 16536;
    constexpr static u32 BINDLESS_TEXTURE_BINDING = 10;


    namespace helper {
        std::string to_string(VkResult result);
        std::string to_string(VkPhysicalDeviceType type);
        /**
        * @brief Check if extensions are available on this device/platform.
        * @param req_exts Extensions required by the application.
        * @return Success: Empty Vector, 
        * @return Failure: Missing Extension(s) list. 
        */
        std::vector<const char*> are_ext_avail(const std::vector<const char*>& req_exts);
        std::vector<VkExtensionProperties> get_extensions();
        /**
        * @brief Check if the layers are available on this device/platform.
        * @param req_layers Layers required by the application.
        * @return Success: Empty Vector,
        * @return Failure: Missing Layers(s) list.
        */
        std::vector<const char*> are_layers_avail(const std::vector<const char*>& req_layers);
        std::vector<VkLayerProperties> get_layers();

        u32 find_mem_type(u32 filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical);
        void transition_image_layout(
            VkCommandBuffer       cmd,
            VkImage               image,
            VkImageLayout*        oldLayout,
            VkImageLayout         newLayout,
            VkAccessFlags2        srcAccessMask,
            VkAccessFlags2        dstAccessMask,
            VkPipelineStageFlags2 srcStage,
            VkPipelineStageFlags2 dstStage
        );
        void transition_image_layout(
            VkCommandBuffer       cmd,
            VkImage               image,
            VkImageLayout         oldLayout,
            VkImageLayout         newLayout,
            VkAccessFlags2        srcAccessMask,
            VkAccessFlags2        dstAccessMask,
            VkPipelineStageFlags2 srcStage,
            VkPipelineStageFlags2 dstStage
        );
        void create_img(
            uint32_t width, uint32_t height,
            VkFormat format, VkImageTiling tiling,
            VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
            VkImage& image, VkDeviceMemory& imageMemory,
            VkDevice device, VkPhysicalDevice physicalDevice
        );
        void copy_buffer_to_img(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void create_img_view(VkDevice device, VkImage image, VkFormat format, VkImageView& view);
        VkCommandBuffer begin_single_time_commands(VkDevice device, VkCommandPool commandPool);
        void end_single_time_commands(VkCommandBuffer commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue);
    }

    namespace pfn {
        bool load_functions(VkInstance instance);
        bool load_functions(VkDevice device);
    }

    void     DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);
    void     DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
    VkResult GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported);
    VkResult GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities);
    VkResult GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats);
    VkResult GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes);
    VkResult GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages);
    VkResult CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain);
    VkResult AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);
    VkResult QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);
    PFN_vkVoidFunction GetDeviceProcAddr(VkDevice device, const char* pName);


}