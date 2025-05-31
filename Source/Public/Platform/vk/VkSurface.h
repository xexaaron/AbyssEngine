#pragma once
#include "Platform/vk/VkCommon.h"
#include "Platform/vk/VkInstance.h"
#include "Platform/vk/VkDeviceManager.h"
#include "Core/Window.h"

namespace aby::vk {

	class Surface {
	public:
		Surface();
		Surface(Instance& instance, Window* window);

		void create(Instance& instance, Window* window);
		void destroy();

		VkSurfaceFormatKHR format(DeviceManager& devices) const;

		operator VkSurfaceKHR();
	private:
		VkSurfaceKHR m_Surface;
		VkSurfaceFormatKHR m_Format;
		VkInstance m_Instance;
	};

}