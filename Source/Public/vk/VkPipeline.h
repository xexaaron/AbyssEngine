#pragma once

#include "vk/VkCommon.h"
#include "vk/VkShader.h"
#include "vk/VkDeviceManager.h"
#include "vk/VkSwapchain.h"
namespace aby::vk {

	class Context;

	class Pipeline {
	public:
		Pipeline();
		Pipeline(Window* window, DeviceManager& manager, Ref<ShaderModule> shaders, Swapchain& swapchain);
		
		void create(Window* window, DeviceManager& manager, Ref<ShaderModule> shaders, Swapchain& swapchain);
		void destroy();

		void bind(VkCommandBuffer buffer);

		Ref<ShaderModule> shaders();
		VkPipelineRenderingCreateInfo create_info();

		operator VkPipeline();
	private:
		VkDevice m_Device;
		Ref<ShaderModule> m_Shaders;
		VkPipeline m_Pipeline;
		VkPipelineRenderingCreateInfo m_CreateInfo;
	};

}