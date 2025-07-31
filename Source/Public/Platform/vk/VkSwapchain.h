#pragma once
#include "Platform/vk/VkCommon.h"
#include "Platform/vk/VkDeviceManager.h"
#include "Platform/vk/VkSurface.h"
#include "Rendering/Window.h"
#include "Core/Common.h"

namespace aby::vk {
	
	struct Frame {
		Frame();
		Frame(DeviceManager& manager);

		void create(DeviceManager& manager);
		void destroy(DeviceManager& manager);

		VkSemaphore     acquire;
		VkSemaphore     release;
		VkFence         queue_submit;
		VkCommandBuffer cmd_buffer;
		VkCommandPool   cmd_pool;
	};

	class Swapchain {
	public:
		Swapchain();
		Swapchain(Surface& surface, DeviceManager& devices, Window* window, std::vector<Frame>& frames);

		void create(Surface& surface, DeviceManager& devices, Window* window, std::vector<Frame>& frames);
		void destroy(DeviceManager& devices, std::vector<Frame>& frames);

		std::vector<VkImageView>& views();
		std::vector<VkImage>& images();

		std::size_t frames_in_flight() const;
		u32 width() const;
		u32 height() const;
		glm::u32vec2  size() const;
		VkFormat format() const;
		operator VkSwapchainKHR();
	private:
		VkSwapchainKHR m_Swapchain;
		VkExtent2D m_Extent;
		VkFormat m_Format;
		std::vector<VkImage> m_Images;
		std::vector<VkImageView>  m_Views;
	};

}