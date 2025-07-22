#include "Platform/vk/VkShaderStructs.h"
#include "Utility/Inserter.h"

namespace aby::vk {

    std::map<std::size_t, std::size_t> ShaderDescriptor::uniform_binding_sizes() const {
        std::map<std::size_t, std::size_t> binding_size_map;
        for (const auto& uniform : uniforms) {
            binding_size_map[uniform.binding] += uniform.size;
        }
        return binding_size_map;
    }
    std::map<std::size_t, std::size_t> ShaderDescriptor::input_binding_sizes() const {
        std::map<std::size_t, std::size_t> binding_size_map;
        for (const auto& input : inputs) {
            binding_size_map[input.binding] += format_size(input.format);
        }
        return binding_size_map;
    }
    std::map<std::size_t, std::size_t> ShaderDescriptor::input_binding_stride() const {
        std::map<std::size_t, std::size_t> binding_stride_map;
        for (const auto& input : inputs) {
            binding_stride_map[input.binding] += format_size(input.format);
        }
        return binding_stride_map;
    }

    std::size_t ShaderDescriptor::format_size(VkFormat format) {
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

}

namespace aby::vk {

    VertexClass::VertexClass(const ShaderDescriptor& descriptor, std::size_t max_vertices, u32 binding) :
        m_Binding(binding),
        m_VertexSize(0),
        m_MaxVertices(max_vertices)
    {
        auto mapped = util::map_vector(descriptor.inputs, [](const ShaderInput& input) { return input.binding; });
        auto range = mapped.equal_range(binding);
        for (auto it = range.first; it != range.second; ++it) {
            ShaderInput& input = it->second;
            m_VertexSize += descriptor.format_size(input.format);
        }
    }

    std::size_t VertexClass::binding() const {
        return m_Binding;
    }

    std::size_t VertexClass::vertex_size() const {
        return m_VertexSize;
    }

    std::size_t VertexClass::max_vertices() const {
        return m_MaxVertices;
    }

}