#include "vk/VkSwapchain.h"
#include "vk/VkAllocator.h"
#include "Core/Log.h"
#include <algorithm>

namespace aby::vk {

	Swapchain::Swapchain() :
		m_Swapchain(VK_NULL_HANDLE),
        m_Extent{ 0, 0 },
        m_Format(VK_FORMAT_UNDEFINED),
        m_Images{},
        m_Views{}
    {

	}

    Swapchain::Swapchain(Surface& surface, DeviceManager& devices, Window* window, std::vector<Frame>& frames) :
        m_Swapchain(VK_NULL_HANDLE),
        m_Extent{ 0, 0 },
        m_Format(VK_FORMAT_UNDEFINED),
        m_Images{},
        m_Views{}
    {
		create(surface, devices, window, frames);
	}
        
    void Swapchain::destroy(DeviceManager& manager, std::vector<Frame>& frames) {
        auto logical = manager.logical();
        vkDeviceWaitIdle(logical);
        for (auto& frame : frames) {
            frame.destroy(manager);
        }
        frames.clear();

        DestroySwapchainKHR(logical, m_Swapchain, IAllocator::get());

        for (auto& view : m_Views) {
            vkDestroyImageView(logical, view, IAllocator::get());
        }
        m_Views.clear();
    }

    std::vector<VkImageView>& Swapchain::views() {
        return m_Views;
    }

    std::vector<VkImage>& Swapchain::images() {
        return m_Images;
    }

    std::uint32_t Swapchain::width() const {
        return m_Extent.width;
    }

    std::uint32_t Swapchain::height() const {
        return m_Extent.height;
    }

   glm::u32vec2 Swapchain::size() const {
       return { m_Extent.width, m_Extent.height };
    }

    VkFormat Swapchain::format() const {
        return m_Format;
    }

	void Swapchain::create(Surface& surface, DeviceManager& devices, Window* window, std::vector<Frame>& frames) {
        auto logical  = devices.logical();
        auto format   = surface.format(devices);
        m_Format      = format.format;
        auto physical = devices.physical();

        VkSurfaceCapabilitiesKHR caps;
        VK_CHECK(GetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &caps));
        std::vector<VkPresentModeKHR> present_modes;
        VK_ENUMERATE(present_modes, GetPhysicalDeviceSurfacePresentModesKHR, physical, surface);

        VkExtent2D swapchain_extent =
            (caps.currentExtent.width != UINT32_MAX)
            ? caps.currentExtent
            : VkExtent2D{ static_cast<uint32_t>(window->width()), static_cast<uint32_t>(window->height()) };

        m_Extent.width  = std::clamp(swapchain_extent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
        m_Extent.height = std::clamp(swapchain_extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);

        VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;

        uint32_t swapchain_img_count = caps.minImageCount + 1;
        if (swapchain_img_count < caps.minImageCount) {
            swapchain_img_count = caps.minImageCount;
        }
        if ((caps.maxImageCount > 0) && (swapchain_img_count > caps.maxImageCount)) {
            swapchain_img_count = caps.maxImageCount;
        }

        VkSurfaceTransformFlagBitsKHR pre_transform;
        if (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
            pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        }
        else {
            pre_transform = caps.currentTransform;
        }
        VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        VkCompositeAlphaFlagBitsKHR composite_alpha_flags[4] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        };
        for (uint32_t i = 0; i < std::size(composite_alpha_flags); i++) {
            if (caps.supportedCompositeAlpha & composite_alpha_flags[i]) {
                composite_alpha = composite_alpha_flags[i];
                break;
            }
        }

        VkSwapchainKHR old_swapchain = m_Swapchain;

        VkSwapchainCreateInfoKHR swapchain_ci = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = surface,
            .minImageCount = swapchain_img_count,
            .imageFormat = format.format,
            .imageColorSpace = format.colorSpace,
            .imageExtent = swapchain_extent, 
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .preTransform = pre_transform,
            .compositeAlpha = composite_alpha,
            .presentMode = present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = old_swapchain,
        };


        VK_CHECK(CreateSwapchainKHR(logical, &swapchain_ci, IAllocator::get(), &m_Swapchain));

        if (old_swapchain != VK_NULL_HANDLE) {
            for (VkImageView view : m_Views) {
                vkDestroyImageView(logical, view, IAllocator::get());
            }
            for (Frame& frame : frames) {
                frame.destroy(devices);
            }

            m_Views.clear();

            DestroySwapchainKHR(logical, old_swapchain, IAllocator::get());
        }

        VK_ENUMERATE(m_Images, GetSwapchainImagesKHR, logical, m_Swapchain);
        
        frames.clear();
        frames.resize(m_Images.size());
        for (std::size_t i = 0; i < m_Images.size(); i++) {
            frames[i] = Frame(devices);
        }
        
        m_Views.resize(m_Images.size());
        for (size_t i = 0; i < m_Images.size(); i++) {
            VkImageViewCreateInfo view_ci = {
               .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
               .pNext = nullptr,
               .flags = 0,
               .image = m_Images[i],
               .viewType = VK_IMAGE_VIEW_TYPE_2D,
               .format = format.format,
               .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
               .subresourceRange = { 
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
            };

            VK_CHECK(vkCreateImageView(logical, &view_ci, IAllocator::get(), &m_Views[i]));
        }
    }

    

    Swapchain::operator VkSwapchainKHR() {
        return m_Swapchain;
    }

}

namespace aby::vk {

    Frame::Frame() :
        acquire(VK_NULL_HANDLE),
        release(VK_NULL_HANDLE),
        queue_submit(VK_NULL_HANDLE),
        cmd_buffer(VK_NULL_HANDLE),
        cmd_pool(VK_NULL_HANDLE) {}

    Frame::Frame(DeviceManager& manager) :
        acquire(VK_NULL_HANDLE),
        release(VK_NULL_HANDLE),
        queue_submit(VK_NULL_HANDLE),
        cmd_buffer(VK_NULL_HANDLE),
        cmd_pool(VK_NULL_HANDLE)
    {
        create(manager);
    }

    void Frame::create(DeviceManager& manager) {
        auto logical = manager.logical();

        VkFenceCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        VK_CHECK(vkCreateFence(logical, &info, IAllocator::get(), &queue_submit));

        VkCommandPoolCreateInfo cmd_pool_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = manager.graphics().FamilyIdx
        };
        VK_CHECK(vkCreateCommandPool(logical, &cmd_pool_info, IAllocator::get(), &cmd_pool));

        VkCommandBufferAllocateInfo cmd_buf_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = cmd_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };
        VK_CHECK(vkAllocateCommandBuffers(logical, &cmd_buf_info, &cmd_buffer));
    }

    void Frame::destroy(DeviceManager& manager) {
        auto logical = manager.logical();
        if (queue_submit != VK_NULL_HANDLE)
        {
            vkDestroyFence(logical, queue_submit, IAllocator::get());
        }

        if (cmd_buffer != VK_NULL_HANDLE)
        {
            vkFreeCommandBuffers(logical, cmd_pool, 1, &cmd_buffer);
            cmd_buffer = VK_NULL_HANDLE;
        }

        if (cmd_pool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(logical, cmd_pool, IAllocator::get());
        }

        if (acquire != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(logical, acquire, IAllocator::get());
        }
        if (release != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(logical, release, IAllocator::get());
        }
    }

}