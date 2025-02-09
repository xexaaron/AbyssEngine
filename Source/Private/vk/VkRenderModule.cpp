#include "vk/VkRenderModule.h"

namespace aby::vk {

    RenderModule::RenderModule(Ref<vk::Context> ctx, vk::Swapchain& swapchain, const std::vector<fs::path>& shaders, std::size_t max_vertices) :
        m_Module(ShaderModule::create(ctx.get(), shaders[0], shaders[1])),
        m_Class(m_Module->vert().descriptor(), max_vertices, 0),
        m_Accumulator(m_Class),
        m_Vertices(m_Class, ctx->devices()),
        m_Pipeline(ctx->window(), ctx->devices(), m_Module, swapchain)
    { }

    void RenderModule::destroy() {
        m_Pipeline.destroy();
        m_Vertices.destroy();
    }

    const vk::VertexClass& RenderModule::vertex_class() const {
        return m_Class;
    }

    Ref<ShaderModule> RenderModule::module() const {
        return m_Module;
    }

    vk::VertexAccumulator& RenderModule::accumulator() {
        return m_Accumulator;
    }

    vk::VertexBuffer& RenderModule::vertices() {
        return m_Vertices;
    }

    vk::Pipeline& RenderModule::pipeline() {
        return m_Pipeline;
    }

}