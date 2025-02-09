#pragma once
#include "vk/VkCommon.h"
#include "vk/VkInstance.h"
#include "Core/Common.h"

namespace aby::vk {
	
	class Debugger {
	public:
		Debugger();
		Debugger(Instance& instance);
		~Debugger();

		void create(Instance& instance);
		void destroy();

		operator VkDebugUtilsMessengerEXT();
	protected:
		static VkBool32 msg_callback(
			VkDebugUtilsMessageSeverityFlagBitsEXT severity,
			VkDebugUtilsMessageTypeFlagsEXT type,
			const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
			void* user_data
		);
	private:
		VkDebugUtilsMessengerEXT m_Debugger;
		VkInstance m_Instance;
	};


}