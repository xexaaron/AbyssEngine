#pragma once
#include "Platform/vk/VkCommon.h"
#include "Platform/vk/VkInstance.h"

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
		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_Debugger;
	};


}