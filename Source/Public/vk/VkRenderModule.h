#pragma once

#include "vk/VkSwapchain.h"
#include "vk/VkPipeline.h"
#include "vk/VkBuffer.h"
#include "vk/VkShader.h"
#include "vk/VkContext.h"
#include "Rendering/Vertex.h"

namespace aby::vk {

    struct PrimitiveDescriptor {
        std::uint32_t MaxVertices;
        std::uint32_t MaxIndices;
        std::uint32_t IndicesPer;
        std::uint32_t VerticesPer;
    };

    class RenderPrimitive {
    public:
        RenderPrimitive(Ref<vk::Context> ctx, const ShaderDescriptor& vertex_descriptor, const PrimitiveDescriptor& primitive_descriptor);

        void destroy();
        void reset();
        void bind(VkCommandBuffer cmd, DeviceManager& manager);
        void draw(VkCommandBuffer cmd);

        void set_index_data(const std::uint32_t* indices, DeviceManager& manager);

        bool should_flush() const;
        bool empty() const;
        std::uint32_t index_count() const;
        std::uint32_t vertex_count() const;
        const vk::PrimitiveDescriptor& descriptor() const;

        RenderPrimitive& operator++() {
            ++m_VertexAccumulator;
            if (m_VertexAccumulator.count() % m_Descriptor.VerticesPer == 0) {
                m_IndexCount += m_Descriptor.IndicesPer;
            }
            return *this;
        }
        template <typename T>
        RenderPrimitive& operator=(const T& data) {
            m_VertexAccumulator = data;
            return *this;
        }
    protected:
        void draw_indexed(VkCommandBuffer cmd);
        void draw_nonindexed(VkCommandBuffer cmd);
    private:
        vk::VertexClass       m_VertexClass;
        vk::VertexAccumulator m_VertexAccumulator;
        vk::VertexBuffer      m_VertexBuffer;
        vk::IndexBuffer       m_IndexBuffer;
        PrimitiveDescriptor   m_Descriptor;
        std::uint32_t         m_IndexCount;
    };

    enum class ERenderPrimitive {
        TRIANGLE = 0,
        QUAD     = 1,
        MAX_ENUM = 2,
        ALL,
    };

    using RenderPrimitiveArray = std::array<RenderPrimitive, static_cast<std::size_t>(ERenderPrimitive::MAX_ENUM)>;

    class RenderModule {
    public:
    public:
        RenderModule(Ref<vk::Context> ctx, vk::Swapchain& swapchain, const std::vector<fs::path>& shaders);

        void destroy();
        void reset();
        void flush(VkCommandBuffer cmd, DeviceManager& manager, ERenderPrimitive primitive = ERenderPrimitive::ALL);
        void set_uniforms(const void* data, std::size_t bytes, std::size_t binding = 0);
        void draw_triangle(const Triangle& triangle);
        void draw_quad(const Quad& quad);

        Ref<ShaderModule> module() const;
        vk::Pipeline&     pipeline();

        RenderPrimitive&  quads();
        RenderPrimitive&  tris();
    private:
        Ref<ShaderModule>     m_Module;
        vk::Pipeline          m_Pipeline;
        RenderPrimitiveArray  m_Primitives;
    };


}