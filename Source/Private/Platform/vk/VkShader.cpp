#include "Platform/vk/VkShader.h"
#include "Platform/vk/VkAllocator.h"
#include "Platform/vk/VkContext.h"
#include "Platform/vk/VkTexture.h"
#include "Core/Log.h"
#include "Core/App.h"

#include <imgui/backends/imgui_impl_vulkan.h>

namespace aby::vk {

    Shader::Shader(App* app, DeviceManager& devices, const fs::path& path, EShader type) :
        aby::Shader(
            ShaderCompiler::compile(app, devices, path, type),
            type == EShader::FROM_EXT ? ShaderCompiler::get_type_from_ext(path.extension()) : type
        ),
        m_Logical(devices.logical()),
        m_Module(VK_NULL_HANDLE),
        m_Layout(VK_NULL_HANDLE),
        m_Descriptor(ShaderCompiler::reflect(m_Data)) 
    {
        Timer timer;
        // Create shader module
        VkShaderModuleCreateInfo smci = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = m_Data.size() * sizeof(u32),
            .pCode = m_Data.data(),
        };
        VK_CHECK(vkCreateShaderModule(m_Logical, &smci, IAllocator::get(), &m_Module));

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkDescriptorBindingFlags> flags;

        VkShaderStageFlags stageFlag = (m_Type == EShader::VERTEX) ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;

        // Process uniform buffers
        for (const auto& uniform : m_Descriptor.uniforms) {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = uniform.binding;
            binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            binding.descriptorCount = 1;
            binding.stageFlags = stageFlag;
            binding.pImmutableSamplers = nullptr;
            bindings.push_back(binding);
            flags.push_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
        }
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = BINDLESS_TEXTURE_BINDING;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = MAX_BINDLESS_RESOURCES;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding.pImmutableSamplers = nullptr;
        bindings.push_back(binding);
        flags.push_back(
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
        );

        // Descriptor binding flags
        VkDescriptorSetLayoutBindingFlagsCreateInfo bfci{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = static_cast<uint32_t>(flags.size()),
            .pBindingFlags = flags.data(),
        };
                
        // Create descriptor set layout
        VkDescriptorSetLayoutCreateInfo layoutInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &bfci,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
        };
        VK_CHECK(vkCreateDescriptorSetLayout(m_Logical, &layoutInfo, IAllocator::get(), &m_Layout));
   
        ABY_LOG("Loaded Shader: {}ms", timer.elapsed().milli());
        ABY_LOG("  Path:     {}", path);
        ABY_LOG("  Type:     {}", std::to_string(type));
        ABY_LOG("  Inputs:   {}", m_Descriptor.inputs.size());
        ABY_LOG("  Uniforms: {}", m_Descriptor.uniforms.size());
        ABY_LOG("  Samplers: {}", m_Descriptor.samplers.size());
        ABY_LOG("  Storages: {}", m_Descriptor.storages.size());
    }


    Ref<Shader> Shader::create(App* app, DeviceManager& devices, const fs::path& path, EShader type) {
        return create_ref<Shader>(app, devices, path, type);
    }

    Shader::~Shader() {
        m_Data.clear();
    }

    void Shader::destroy() {
        vkDestroyShaderModule(m_Logical, m_Module, IAllocator::get());
        vkDestroyDescriptorSetLayout(m_Logical, m_Layout, IAllocator::get());
    }

    const ShaderDescriptor& Shader::descriptor() const {
        return m_Descriptor;
    }

    VkDescriptorSetLayout Shader::layout() const {
        return m_Layout;
    }
    
    VkPipelineShaderStageCreateInfo Shader::stage() const {
        VkShaderStageFlagBits stage_flags;
        switch (m_Type) {
            case EShader::VERTEX:
                stage_flags = VK_SHADER_STAGE_VERTEX_BIT;
                break;
            case EShader::FRAGMENT:
                stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT;
                break;
            default:
                throw std::out_of_range("EShader");
        }
        VkPipelineShaderStageCreateInfo ci = {
           .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
           .pNext = nullptr,
           .flags = 0,
           .stage = stage_flags,
           .module = m_Module,
           .pName = "main",  // Entry point name
           .pSpecializationInfo = nullptr,
        };
        return ci;
    }

    Shader::operator VkShaderModule() const {
        return m_Module;
    }   


}
