#pragma once
#include "Core/Common.h"
#include "vk/VkCommon.h"
#include "vk/VkDeviceManager.h"
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
        std::map<std::size_t, std::size_t> uniform_binding_sizes() const {
            std::map<std::size_t, std::size_t> binding_size_map;
            for (const auto& uniform : uniforms) {
                binding_size_map[uniform.binding] += uniform.size;
            }
            return binding_size_map;
        }
        std::map<std::size_t, std::size_t> input_binding_sizes() const {
            std::map<std::size_t, std::size_t> binding_size_map;
            for (const auto& input : inputs) {
                binding_size_map[input.binding] += format_size(input.format);
            }
            return binding_size_map;
        }
        std::map<std::size_t, std::size_t> input_binding_stride() const {
            std::map<std::size_t, std::size_t> binding_stride_map;
            for (const auto& input : inputs) {
                binding_stride_map[input.binding] += format_size(input.format);
            }
            return binding_stride_map;
        }

        static constexpr std::size_t format_size(VkFormat format) {
            switch (format) {
                case VK_FORMAT_R32_SFLOAT: return 4;
                case VK_FORMAT_R32G32_SFLOAT: return 8;
                case VK_FORMAT_R32G32B32_SFLOAT: return 12;
                case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
                case VK_FORMAT_R32_SINT: return 4;
                case VK_FORMAT_R32G32_SINT: return 8;
                case VK_FORMAT_R32G32B32_SINT: return 12;
                case VK_FORMAT_R32G32B32A32_SINT: return 16;
                case VK_FORMAT_R32_UINT: return 4;
                case VK_FORMAT_R32G32_UINT: return 8;
                case VK_FORMAT_R32G32B32_UINT: return 12;
                case VK_FORMAT_R32G32B32A32_UINT: return 16;
                default: return 0;
            }
        }

        std::vector<ShaderUniform> uniforms;
        std::vector<ShaderStorage> storages;
        std::vector<ShaderSampler> samplers;
        std::vector<ShaderInput>   inputs;
    };
    
    class VertexClass {
    public:
        VertexClass(const ShaderDescriptor& descriptor, std::size_t max_vertices, std::uint32_t binding = 0) :
            m_Binding(binding),
            m_VertexSize(0),
            m_MaxVertices(max_vertices) 
        {
            auto mapped = map_vector(descriptor.inputs, [](const ShaderInput& input) { return input.binding; });
            auto range  = mapped.equal_range(binding);
            for (auto it = range.first; it != range.second; ++it) {
                ShaderInput& input = it->second;
                m_VertexSize += descriptor.format_size(input.format);
            }
        }

        std::size_t binding() const {
            return m_Binding;
        }

        std::size_t vertex_size() const {
            return m_VertexSize;
        }

        std::size_t max_vertices() const {
            return m_MaxVertices;
        }
    private:
        std::size_t m_Binding;
        std::size_t m_VertexSize;
        std::size_t m_MaxVertices;
    };

}

namespace aby::vk {

    class Context;

    class Shader : public aby::Shader {
    public:
        Shader(DeviceManager& devices, const fs::path& path, EShader type = EShader::FROM_EXT);
        ~Shader();

        static Ref<Shader> create(DeviceManager& devices, const fs::path& path, EShader type);
        
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

        void set_uniforms(const void* data, std::size_t bytes, std::uint32_t binding = 0);
        void update_descriptor_set(std::uint32_t binding, std::size_t bytes);
        void update_uniform_memory(const void* data, std::size_t bytes);

        Resource vert() const;
        Resource frag() const;
        const VertexClass& vertex_class() const;
        const ShaderDescriptor& vertex_descriptor() const;

        VkPipelineLayout layout() const;
        const std::vector<VkDescriptorSet>& descriptors() const;
        std::vector<VkDescriptorSet>& descriptors();

        std::vector<VkPipelineShaderStageCreateInfo> stages() const;
    protected:
        void create_uniform_buffer(VkDeviceSize size);
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

    class ShaderCompiler {
    public:
        static std::vector<std::uint32_t> compile(DeviceManager& devices, const fs::path& path, EShader type = EShader::FROM_EXT);
        static EShader get_type_from_ext(const fs::path& ext);
        static fs::path cache_dir(const fs::path& file = "");
        static ShaderDescriptor reflect(const std::vector<std::uint32_t>& binary_data);
    private:
    };
}
