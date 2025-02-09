#include "vk/VkCommon.h"
#include "vk/VkAllocator.h"
#include "Core/Log.h"

namespace aby::vk::helper {

    std::string to_string(VkResult result) {
        switch (result) {
            case VK_SUCCESS:                                 return "VK_SUCCESS";
            case VK_NOT_READY:                               return "VK_NOT_READY";
            case VK_TIMEOUT:                                 return "VK_TIMEOUT";
            case VK_EVENT_SET:                               return "VK_EVENT_SET";
            case VK_EVENT_RESET:                             return "VK_EVENT_RESET";
            case VK_INCOMPLETE:                              return "VK_INCOMPLETE";
            case VK_ERROR_OUT_OF_HOST_MEMORY:                return "VK_ERROR_OUT_OF_HOST_MEMORY";
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:              return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            case VK_ERROR_INITIALIZATION_FAILED:             return "VK_ERROR_INITIALIZATION_FAILED";
            case VK_ERROR_DEVICE_LOST:                       return "VK_ERROR_DEVICE_LOST";
            case VK_ERROR_MEMORY_MAP_FAILED:                 return "VK_ERROR_MEMORY_MAP_FAILED";
            case VK_ERROR_LAYER_NOT_PRESENT:                 return "VK_ERROR_LAYER_NOT_PRESENT";
            case VK_ERROR_EXTENSION_NOT_PRESENT:             return "VK_ERROR_EXTENSION_NOT_PRESENT";
            case VK_ERROR_FEATURE_NOT_PRESENT:               return "VK_ERROR_FEATURE_NOT_PRESENT";
            case VK_ERROR_INCOMPATIBLE_DRIVER:               return "VK_ERROR_INCOMPATIBLE_DRIVER";
            case VK_ERROR_TOO_MANY_OBJECTS:                  return "VK_ERROR_TOO_MANY_OBJECTS";
            case VK_ERROR_FORMAT_NOT_SUPPORTED:              return "VK_ERROR_FORMAT_NOT_SUPPORTED";
            case VK_ERROR_FRAGMENTED_POOL:                   return "VK_ERROR_FRAGMENTED_POOL";
            case VK_ERROR_UNKNOWN:                           return "VK_ERROR_UNKNOWN";
            case VK_ERROR_OUT_OF_POOL_MEMORY:                return "VK_ERROR_OUT_OF_POOL_MEMORY";
            case VK_ERROR_INVALID_EXTERNAL_HANDLE:           return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            case VK_ERROR_FRAGMENTATION:                     return "VK_ERROR_FRAGMENTATION";
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:    return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            case VK_PIPELINE_COMPILE_REQUIRED:               return "VK_PIPELINE_COMPILE_REQUIRED";
            case VK_ERROR_SURFACE_LOST_KHR:                  return "VK_ERROR_SURFACE_LOST_KHR";
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:          return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            case VK_SUBOPTIMAL_KHR:                          return "VK_SUBOPTIMAL_KHR";
            case VK_ERROR_OUT_OF_DATE_KHR:                   return "VK_ERROR_OUT_OF_DATE_KHR";
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:          return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            case VK_ERROR_VALIDATION_FAILED_EXT:             return "VK_ERROR_VALIDATION_FAILED_EXT";
            case VK_ERROR_INVALID_SHADER_NV:                 return "VK_ERROR_INVALID_SHADER_NV";
            case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:     return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
            case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
            case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            case VK_THREAD_IDLE_KHR:                         return "VK_THREAD_IDLE_KHR";
            case VK_THREAD_DONE_KHR:                         return "VK_THREAD_DONE_KHR";
            case VK_OPERATION_DEFERRED_KHR:                  return "VK_OPERATION_DEFERRED_KHR";
            case VK_OPERATION_NOT_DEFERRED_KHR:              return "VK_OPERATION_NOT_DEFERRED_KHR";
            case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:         return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
            default:                                          return "Unknown VkResult";
        }
    }
     
    std::string to_string(VkPhysicalDeviceType type) {
        switch (type) {
            case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU:
                return "cpu";
            case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                return "discrete GPU";
            case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                return "integrated GPU";
            case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                return "virtual GPU";
            case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_OTHER:
                return "other";
            default:
                return "VkPhysicalDeviceType out of range";
        }
        return "UNREACHABLE";
    }
    std::vector<VkExtensionProperties> get_extensions() {
        std::vector<VkExtensionProperties> extensions;
        VK_ENUMERATE(extensions, vkEnumerateInstanceExtensionProperties, nullptr);
        return extensions;
    }

    std::vector<const char*> are_ext_avail(const std::vector<const char*>& req_exts) {
        std::vector<VkExtensionProperties> extensions = get_extensions();
        std::vector<const char*> missing_exts;

        for (std::size_t i = 0; i < req_exts.size(); i++) {
            bool found = false;
            for (const auto& extension : extensions) {
                if (strcmp(req_exts[i], extension.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                missing_exts.push_back(req_exts[i]);
            }
        }
        return missing_exts;
    }

    std::vector<VkLayerProperties> get_layers() {
        std::uint32_t layer_count = 0;
        VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, nullptr));
        std::vector<VkLayerProperties> layers(layer_count);
        VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, layers.data()));
        return layers;
    }

    std::vector<const char*> are_layers_avail(const std::vector<const char*>& req_layers) {
        auto layers = get_layers();
        std::vector<const char*> missing_layers;
        for (std::size_t i = 0; i < req_layers.size(); i++) {
            bool found = false;
            for (const auto& layer : layers) {
                if (strcmp(req_layers[i], layer.layerName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                missing_layers.push_back(req_layers[i]);
            }
        }
        return missing_layers;
    }

    std::uint32_t find_mem_type(std::uint32_t filter, VkMemoryPropertyFlags properties, VkPhysicalDevice physical) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physical, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("Failed to find a suitable memory type!");
    }

    void transition_image_layout(
        VkCommandBuffer       cmd,
        VkImage               image,
        VkImageLayout*        oldLayout,
        VkImageLayout         newLayout,
        VkAccessFlags2        srcAccessMask,
        VkAccessFlags2        dstAccessMask,
        VkPipelineStageFlags2 srcStage,
        VkPipelineStageFlags2 dstStage
    ) {
        // Initialize the VkImageMemoryBarrier2 structure
        VkImageMemoryBarrier2 image_barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

            // Specify the pipeline stages and access masks for the barrier
            .srcStageMask = srcStage,             // Source pipeline stage mask
            .srcAccessMask = srcAccessMask,        // Source access mask
            .dstStageMask = dstStage,             // Destination pipeline stage mask
            .dstAccessMask = dstAccessMask,        // Destination access mask

            // Specify the old and new layouts of the image
            .oldLayout = *oldLayout,        // Current layout of the image
            .newLayout = newLayout,        // Target layout of the image

            // We are not changing the ownership between queues
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

            // Specify the image to be affected by this barrier
            .image = image,

            // Define the subresource range (which parts of the image are affected)
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,        // Affects the color aspect of the image
                .baseMipLevel = 0,                                // Start at mip level 0
                .levelCount = 1,                                // Number of mip levels affected
                .baseArrayLayer = 0,                                // Start at array layer 0
                .layerCount = 1                                 // Number of array layers affected
            } };

        // Initialize the VkDependencyInfo structure
        VkDependencyInfo dependency_info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = 0,                    // No special dependency flags
            .imageMemoryBarrierCount = 1,                    // Number of image memory barriers
            .pImageMemoryBarriers = &image_barrier        // Pointer to the image memory barrier(s)
        };

        vkCmdPipelineBarrier2(cmd, &dependency_info);

        *oldLayout = newLayout;
    }

    void transition_image_layout(
        VkCommandBuffer       cmd,
        VkImage               image,
        VkImageLayout         oldLayout,
        VkImageLayout         newLayout,
        VkAccessFlags2        srcAccessMask,
        VkAccessFlags2        dstAccessMask,
        VkPipelineStageFlags2 srcStage,
        VkPipelineStageFlags2 dstStage
    ) 
    {
        transition_image_layout(cmd, image, &oldLayout, newLayout, srcAccessMask, dstAccessMask, srcStage, dstStage);
    }

    void create_img(
        uint32_t width, uint32_t height, 
        VkFormat format, VkImageTiling tiling, 
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, 
        VkImage& image, VkDeviceMemory& imageMemory, 
        VkDevice device, VkPhysicalDevice physicalDevice)
    {
        // Step 1: Create the Vulkan Image
        VkImageCreateInfo imageCreateInfo = {};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D; // 2D image for textures
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;  // undefined layout at creation
        imageCreateInfo.usage = usage;  // e.g., transfer destination, sampled
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;  // not sharing between queues
        imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;  // no multisampling
        imageCreateInfo.flags = 0;  // no flags

        // Create the image in Vulkan
        VK_CHECK(vkCreateImage(device, &imageCreateInfo, IAllocator::get(), &image));

        // Step 2: Allocate Memory for the Image
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = helper::find_mem_type(memRequirements.memoryTypeBits, properties, physicalDevice);

        // Allocate memory for the image
        VK_CHECK(vkAllocateMemory(device, &allocInfo, IAllocator::get(), &imageMemory));

        // Step 3: Bind the Image Memory
        vkBindImageMemory(device, image, imageMemory, 0);
    }

    void copy_buffer_to_img(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0; // Tightly packed
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(
            cmd,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
    }

    void create_img_view(VkDevice device, VkImage image, VkFormat format, VkImageView& view) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(device, &viewInfo, IAllocator::get(), &view));
    }

    VkCommandBuffer begin_single_time_commands(VkDevice device, VkCommandPool commandPool) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void end_single_time_commands(VkCommandBuffer commandBuffer, VkDevice device, VkCommandPool commandPool, VkQueue queue) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }
}


namespace aby::vk::pfn {
    VK_DEF_PROC(vkCreateDebugUtilsMessengerEXT);
    VK_DEF_PROC(vkDestroyDebugUtilsMessengerEXT);
    VK_DEF_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);
    VK_DEF_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_DEF_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_DEF_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
    VK_DEF_PROC(vkGetSwapchainImagesKHR);
    VK_DEF_PROC(vkGetDeviceProcAddr);
    VK_DEF_PROC(vkCreateSwapchainKHR);
    VK_DEF_PROC(vkDestroySwapchainKHR);
    VK_DEF_PROC(vkAcquireNextImageKHR);
    VK_DEF_PROC(vkQueuePresentKHR);

    bool load_functions(VkInstance instance) {
        VK_LOAD_INST_PROC(instance, vkCreateDebugUtilsMessengerEXT);
        VK_LOAD_INST_PROC(instance, vkDestroyDebugUtilsMessengerEXT);
        VK_LOAD_INST_PROC(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
        VK_LOAD_INST_PROC(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
        VK_LOAD_INST_PROC(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
        VK_LOAD_INST_PROC(instance, vkGetPhysicalDeviceSurfacePresentModesKHR);
        VK_LOAD_INST_PROC(instance, vkGetSwapchainImagesKHR);
        VK_LOAD_INST_PROC(instance, vkGetDeviceProcAddr);
        return true;
    }
    bool load_functions(VkDevice device) {
        VK_LOAD_DEV_PROC(device, vkGetSwapchainImagesKHR);
        VK_LOAD_DEV_PROC(device, vkGetDeviceProcAddr);
        VK_LOAD_DEV_PROC(device, vkCreateSwapchainKHR);
        VK_LOAD_DEV_PROC(device, vkDestroySwapchainKHR);
        VK_LOAD_DEV_PROC(device, vkGetSwapchainImagesKHR);
        VK_LOAD_DEV_PROC(device, vkAcquireNextImageKHR);
        VK_LOAD_DEV_PROC(device, vkQueuePresentKHR);
        return true;
    }

}

namespace aby::vk {
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
        return pfn::vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
        pfn::vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
    }

    VkResult GetPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32* pSupported) {
        return pfn::vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, pSupported);
    }

    VkResult GetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* pSurfaceCapabilities) {
        return pfn::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, pSurfaceCapabilities);
    }

    VkResult GetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pSurfaceFormatCount, VkSurfaceFormatKHR* pSurfaceFormats) {
        return pfn::vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
    }

    VkResult GetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t* pPresentModeCount, VkPresentModeKHR* pPresentModes) {
        return pfn::vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, pPresentModeCount, pPresentModes);
    }

    VkResult GetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) {
        return pfn::vkGetSwapchainImagesKHR(device, swapchain, pSwapchainImageCount, pSwapchainImages);
    }

    PFN_vkVoidFunction GetDeviceProcAddr(VkDevice device, const char* pName) {
        return pfn::vkGetDeviceProcAddr(device, pName);
    }

    VkResult CreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
        return pfn::vkCreateSwapchainKHR(device, pCreateInfo, pAllocator, pSwapchain);
    }

    void DestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) {
        pfn::vkDestroySwapchainKHR(device, swapchain, pAllocator);
    }

    VkResult AcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) {
        return pfn::vkAcquireNextImageKHR(device, swapchain, timeout, semaphore, fence, pImageIndex);
    }

    VkResult QueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
        return pfn::vkQueuePresentKHR(queue, pPresentInfo);
    }
}