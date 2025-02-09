#include "vk/VkPipeline.h"
#include "vk/VkAllocator.h"
#include <array>
#include "Core/Log.h"

namespace aby::vk {
	
	Pipeline::Pipeline() :
		m_Device(VK_NULL_HANDLE),
		m_Shaders(nullptr),
		m_Pipeline(VK_NULL_HANDLE)
	{
	}

	Pipeline::Pipeline(Ref<Window> window, DeviceManager& manager, Ref<ShaderModule> shaders, Swapchain& swapchain) : 
		m_Device(VK_NULL_HANDLE),
		m_Shaders(nullptr),
		m_Pipeline(VK_NULL_HANDLE)
	{
		create(window, manager, shaders, swapchain);
	}

	void Pipeline::create(Ref<Window> window, DeviceManager& manager, Ref<ShaderModule> shaders, Swapchain& swapchain) {
		m_Device   = manager.logical();
		m_Shaders  = shaders;
		m_Pipeline = VK_NULL_HANDLE;

		auto& descriptor = m_Shaders->vert().descriptor();
		
		auto input_binding_stride = descriptor.input_binding_stride();

		std::vector<VkVertexInputBindingDescription> ibds;
		ibds.reserve(input_binding_stride.size());
		for (const auto& [binding, stride] : input_binding_stride) {
			VkVertexInputBindingDescription ibd = {
				.binding   = static_cast<std::uint32_t>(binding),
				.stride    = static_cast<std::uint32_t>(stride),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
			};
			ibds.push_back(ibd);
		}

		std::vector<VkVertexInputAttributeDescription> iads;
		iads.reserve(descriptor.inputs.size());
		for (auto& input : descriptor.inputs) {
			VkVertexInputAttributeDescription iad{
				.location = input.location,
				.binding  = input.binding,
				.format   = input.format,
				.offset   = input.offset
			};
			iads.push_back(iad);
		}

		VkPipelineVertexInputStateCreateInfo vertex_input{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.vertexBindingDescriptionCount   = static_cast<std::uint32_t>(ibds.size()),
			.pVertexBindingDescriptions      = ibds.data(),
			.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(iads.size()),
			.pVertexAttributeDescriptions    = iads.data(),
		};

		VkPipelineInputAssemblyStateCreateInfo input_asm{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};

		VkPipelineRasterizationStateCreateInfo rasterizer{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1.0f
		};

		std::vector<VkDynamicState> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_CULL_MODE,
			VK_DYNAMIC_STATE_FRONT_FACE,
			VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
		};

		VkPipelineColorBlendAttachmentState color_blend_attach{
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		};

		VkPipelineColorBlendStateCreateInfo color_blend_state{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = 1,
			.pAttachments = &color_blend_attach,
		};

		VkPipelineViewportStateCreateInfo viewport{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1 
		};
		VkPipelineDepthStencilStateCreateInfo depth_stencil{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthCompareOp = VK_COMPARE_OP_ALWAYS
		};
		VkPipelineMultisampleStateCreateInfo multisample{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT 
		};
		VkPipelineDynamicStateCreateInfo dynamic_state_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
			.pDynamicStates = dynamic_states.data()
		};

		auto format = swapchain.format();

		VkPipelineRenderingCreateInfo pipeline_rendering_info{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &format,
		};

		std::vector<VkPipelineShaderStageCreateInfo> stages = m_Shaders->stages();

		// Pipeline creation
		VkGraphicsPipelineCreateInfo pipeline_ci{
			.sType				 = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext				 = &pipeline_rendering_info,
			.stageCount			 = static_cast<std::uint32_t>(stages.size()),
			.pStages			 = stages.data(),
			.pVertexInputState	 = &vertex_input,
			.pInputAssemblyState = &input_asm,
			.pViewportState		 = &viewport,
			.pRasterizationState = &rasterizer,
			.pMultisampleState	 = &multisample,
			.pDepthStencilState  = &depth_stencil,
			.pColorBlendState    = &color_blend_state,
			.pDynamicState       = &dynamic_state_info,
			.layout              = m_Shaders->layout(),
			.renderPass			 = VK_NULL_HANDLE,                 
			.subpass			 = 0,
		};

		// Create pipeline
		VK_CHECK(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipeline_ci, IAllocator::get(), &m_Pipeline));
	}


	void Pipeline::destroy() {
		if (m_Pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(m_Device, m_Pipeline, IAllocator::get());
		}
		
		m_Shaders->destroy();
	}

	void Pipeline::bind(VkCommandBuffer buffer) {
		vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
		auto& descriptors = m_Shaders->descriptors();
		vkCmdBindDescriptorSets(
			buffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_Shaders->layout(),      // Pipeline layout
			0,						  // First set index
			static_cast<std::uint32_t>(descriptors.size()),					      // Number of sets
			descriptors.data(),			  // The allocated descriptor set
			0,
			nullptr
		);
	}


	Ref<ShaderModule> Pipeline::shaders() {
		return m_Shaders;
	}

	Pipeline::operator VkPipeline() {
		return m_Pipeline;
	}

}