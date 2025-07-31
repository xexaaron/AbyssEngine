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

    const static std::size_t MAX_FRAMES_IN_FLIGHT     = 3;
    constexpr static u32     MAX_BINDLESS_RESOURCES   = 16536;
    constexpr static u32     BINDLESS_TEXTURE_BINDING = 10;

    namespace helper {
        auto to_string(VkResult result) -> std::string;
        auto to_string(VkPhysicalDeviceType type) -> std::string;
        auto get_extensions() -> std::vector<VkExtensionProperties>;
        auto get_layers() -> std::vector<VkLayerProperties>;
        auto are_ext_avail(const std::vector<const char*>& req_exts) -> std::vector<const char*>;
        auto are_layers_avail(const std::vector<const char*>& req_layers) -> std::vector<const char*>;
        auto find_mem_type(u32 filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical) -> u32;
        auto transition_image_layout(VkCommandBuffer cmd, VkImage image, VkImageLayout* oldLayout, VkImageLayout newLayout, VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask, VkPipelineStageFlags2 srcStage, VkPipelineStageFlags2 dstStage) -> void;
        auto transition_image_layout(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags2 srcAccessMask, VkAccessFlags2 dstAccessMask, VkPipelineStageFlags2 srcStage, VkPipelineStageFlags2 dstStage) -> void;
        auto copy_buffer_to_img(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) -> void;
        auto create_img(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkDevice device, VkPhysicalDevice physicalDevice) -> void;
        auto create_img_view(VkDevice device, VkImage image, VkFormat format, VkImageView& view) -> void;
        auto begin_single_time_commands(VkDevice device, VkCommandPool commandPool) -> VkCommandBuffer;
        auto end_single_time_commands(VkCommandBuffer commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue) -> void;
        auto set_debug_name(VkDevice device, uint64_t handle, VkObjectType type, const char* name) -> void;
        auto set_debug_name(VkDevice device, VkImage image, const char* name) -> void;
        auto set_debug_name(VkDevice device, VkBuffer buffer, const char* name) -> void;
        auto set_debug_name(VkDevice device, VkDescriptorSet set, const char* name) -> void;
        auto set_debug_name(VkDevice device, VkImageView view, const char* name) -> void;

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
    void SetDebugUtilsObjectNameEXT(VkDevice device, VkDebugUtilsObjectNameInfoEXT* info);

}