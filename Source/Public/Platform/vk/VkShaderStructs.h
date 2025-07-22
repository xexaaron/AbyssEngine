#pragma once
#include "Platform/vk/VkCommon.h"

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