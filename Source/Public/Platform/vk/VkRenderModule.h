#pragma once

#include "Platform/vk/VkSwapchain.h"
#include "Platform/vk/VkPipeline.h"
#include "Platform/vk/VkBuffer.h"
#include "Platform/vk/VkShader.h"
#include "Platform/vk/VkContext.h"
#include "Rendering/Vertex.h"
#include <array>

namespace aby::vk {

    struct PrimitiveDescriptor {
        u32 MaxVertices;
        u32 MaxIndices;
        u32 IndicesPer;
        u32 VerticesPer;
    };

    class RenderPrimitive {
    public:
        RenderPrimitive(Ref<vk::Context> ctx, const ShaderDescriptor& vertex_descriptor, const PrimitiveDescriptor& primitive_descriptor);

        void destroy();
        void reset();
        void bind(VkCommandBuffer cmd, DeviceManager& manager);
        void draw(VkCommandBuffer cmd);

        void set_index_data(const u32* indices, DeviceManager& manager);

        bool should_flush() const;
        bool should_flush(std::size_t requested_primitives) const;
        bool empty() const;
        std::size_t index_count() const;
        std::size_t vertex_count() const;
        const vk::PrimitiveDescriptor& descriptor() const;

        RenderPrimitive& operator++();

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
        std::size_t           m_IndexCount;
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
        RenderModule(Ref<vk::Context> ctx, vk::Swapchain& swapchain, const std::vector<fs::path>& shaders);
        RenderModule(Ref<vk::Context> ctx, vk::Swapchain& swapchain, Ref<ShaderModule> module);

        void destroy();
        void reset();
        void flush(VkCommandBuffer cmd, DeviceManager& manager, ERenderPrimitive primitive = ERenderPrimitive::ALL);
        void set_uniforms(const void* data, std::size_t bytes, u32 binding = 0);
        
        void draw_triangle(const Triangle& triangle);
        void draw_quad(const Quad& quad);
        void draw_cube(const Quad& quad);
        void draw_text(const Text& text);

        Ref<ShaderModule> module() const;
        vk::Pipeline&     pipeline();
        RenderPrimitive&  quads();
        RenderPrimitive&  tris();
    private:
        void init();
    private:
        vk::Context*          m_Ctx;
        Ref<ShaderModule>     m_Module;
        vk::Pipeline          m_Pipeline;
        RenderPrimitiveArray  m_Primitives;
    };


}