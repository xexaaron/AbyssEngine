#pragma once
#include "vk/VkCommon.h"
#include "vk/VkInstance.h"
#include "vk/VkDeviceManager.h"
#include "Core/Common.h"

namespace aby::vk {

	class Surface {
	public:
		Surface();
		Surface(Instance& instance, Ref<Window> window);

		void create(Instance& instance, Ref<Window> window);
		void destroy();

		VkSurfaceFormatKHR format(DeviceManager& devices) const;

		operator VkSurfaceKHR();
	private:
		VkSurfaceKHR m_Surface;
		VkSurfaceFormatKHR m_Format;
		VkInstance m_Instance;
	};

}