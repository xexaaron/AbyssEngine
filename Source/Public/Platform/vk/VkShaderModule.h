#pragma once
#include "Platform/vk/VkShader.h"
#include "Platform/vk/VkShaderStructs.h"

namespace aby::vk {

    class TextureResourceHandler;
    class Context;

    class ShaderModule {
    public:
        ShaderModule(vk::Context* ctx, const fs::path& vert, const fs::path& frag);

        static Ref<ShaderModule> create(vk::Context* ctx, const fs::path& vert, const fs::path& frag);
        void destroy();

        void set_uniforms(const void* data, std::size_t bytes, u32 binding = 0);
        void update_descriptor_set(u32 binding, std::size_t bytes);
        void update_uniform_memory(const void* data, std::size_t bytes);

        Resource vert() const;
        Resource frag() const;
        const VertexClass& vertex_class() const;
        const ShaderDescriptor& vertex_descriptor() const;
        VkPipelineLayout layout() const;
        const std::vector<VkDescriptorSet>& descriptors() const;
        std::vector<VkDescriptorSet>& descriptors();
        VkDescriptorPool pool();
        std::vector<VkPipelineShaderStageCreateInfo> stages() const;
    protected:
        void create_uniform_buffer(std::size_t size);
    private:
        vk::Context* m_Ctx;
        VkPipelineLayout m_Layout;
        Resource m_Vertex;
        Resource m_Fragment;
        VkDescriptorPool m_Pool;
        std::vector<VkDescriptorSet> m_Descriptors;
        VkBuffer m_Uniforms;
        VkDeviceMemory m_UniformMemory;
        VertexClass m_Class;
        friend class TextureResourceHandler;
    };

}