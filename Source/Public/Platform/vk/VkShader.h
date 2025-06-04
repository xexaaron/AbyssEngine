#pragma once
#include "Core/Common.h"
#include "Platform/vk/VkCommon.h"
#include "Platform/vk/VkDeviceManager.h"
#include "Rendering/Shader.h"
#include <map>
#include <filesystem>

namespace aby::vk {

    struct ShaderUniform {
        std::string name;
        uint32_t set;
        uint32_t binding;
        uint32_t size;
    };
    struct ShaderInput {
        uint32_t location;
        uint32_t binding;
        uint32_t offset;
        uint32_t stride;
        VkFormat format;
    };
    struct ShaderStorage {
        std::string name;
        uint32_t set;
        uint32_t binding;
    };
    struct ShaderSampler {
        std::string name;
        uint32_t set;
        uint32_t binding;
        uint32_t count;
    };
    
    struct ShaderDescriptor {
        std::map<std::size_t, std::size_t> uniform_binding_sizes() const;
        std::map<std::size_t, std::size_t> input_binding_sizes() const;
        std::map<std::size_t, std::size_t> input_binding_stride() const;
        static std::size_t format_size(VkFormat format);

        std::vector<ShaderUniform> uniforms;
        std::vector<ShaderStorage> storages;
        std::vector<ShaderSampler> samplers;
        std::vector<ShaderInput>   inputs;
    };
    
    class VertexClass {
    public:
        VertexClass(const ShaderDescriptor& descriptor, std::size_t max_vertices, u32 binding = 0);
        
        std::size_t binding() const;
        std::size_t vertex_size() const;
        std::size_t max_vertices() const;
    private:
        std::size_t m_Binding;
        std::size_t m_VertexSize;
        std::size_t m_MaxVertices;
    };

}

namespace aby {
    class App;
}

namespace aby::vk {

    class Context;

    class Shader : public aby::Shader {
    public:
        Shader(App* app, DeviceManager& devices, const fs::path& path, EShader type = EShader::FROM_EXT);
        ~Shader();

        static Ref<Shader> create(aby::App* app, DeviceManager& devices, const fs::path& path, EShader type);

        void destroy();

        const ShaderDescriptor& descriptor() const;
        VkDescriptorSetLayout layout() const;
        VkPipelineShaderStageCreateInfo stage() const;

        operator VkShaderModule() const;
    private:
        VkDevice m_Logical;
        VkShaderModule m_Module;
        VkDescriptorSetLayout m_Layout;
        ShaderDescriptor m_Descriptor;
    };

    class TextureResourceHandler;

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



namespace aby::vk {

    class ShaderCompiler {
    public:
        static std::vector<u32> compile(App* app, DeviceManager& devices, const fs::path& path, EShader type = EShader::FROM_EXT);
        static EShader get_type_from_ext(const fs::path& ext);
        static fs::path cache_dir(App* app, const fs::path& file = "");
        static ShaderDescriptor reflect(const std::vector<u32>& binary_data);
    private:
    };
}
