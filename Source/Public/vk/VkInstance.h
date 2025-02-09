#pragma once
#include "vk/VkCommon.h"
#include "Core/Window.h"
#include "Core/Common.h"


namespace aby::vk {
	
	class Instance {
	public:
		Instance();
		Instance(const AppInfo& info, const std::vector<const char*>& extensions, const std::vector<const char*>& layers);
		~Instance();

		void create(const AppInfo& info, const std::vector<const char*>& extensions, const std::vector<const char*>& layers);
		void destroy();

		VkInstance inst();
		operator VkInstance();
	private:
		VkInstance m_Inst;
	};

}