#pragma once

#include "vk/VkSwapchain.h"
#include "vk/VkPipeline.h"
#include "vk/VkBuffer.h"
#include "vk/VkShader.h"
#include "vk/VkContext.h"

namespace aby::vk {

    class RenderModule {
    public:
        RenderModule(Ref<vk::Context> ctx, vk::Swapchain& swapchain, const std::vector<fs::path>& shaders, std::size_t max_vertices);

        void destroy();

        const vk::VertexClass& vertex_class() const;
        Ref<ShaderModule> module() const;
        vk::VertexAccumulator& accumulator();
        vk::VertexBuffer& vertices();
        vk::Pipeline& pipeline();
    private:
        Ref<ShaderModule>     m_Module;
        vk::VertexClass       m_Class;     
        vk::VertexAccumulator m_Accumulator;
        vk::VertexBuffer      m_Vertices;
        vk::Pipeline          m_Pipeline;
    };

}