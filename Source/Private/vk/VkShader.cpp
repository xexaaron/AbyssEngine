#include "vk/VkShader.h"
#include "vk/VkAllocator.h"
#include "vk/VkContext.h"
#include "vk/VkTexture.h"
#include "Core/Log.h"
#include "Core/App.h"
#include <set>
#include <fstream>

#pragma warning(push, 0)
    #include <shaderc/shaderc.hpp>
    #include <spirv_cross/spirv_cross.hpp>
    #include <spirv_cross/spirv_glsl.hpp>
    #include <spirv_cross/spirv_reflect.hpp>
#pragma warning(pop)

namespace aby::vk::helper {

    shaderc_shader_kind get_shader_type(EShader type) {
        switch (type) {
            case EShader::VERTEX:
                return shaderc_shader_kind::shaderc_glsl_vertex_shader;
            case EShader::FRAGMENT:
                return shaderc_shader_kind::shaderc_glsl_fragment_shader;
            default:
                throw std::out_of_range("EShader");
        }
        throw std::out_of_range("Unreachable Code");
    }

    uint32_t get_stride(const spirv_cross::SPIRType& type) {
        uint32_t scalarSize = 0;

        // Determine the size of a single scalar
        switch (type.basetype) {
            case spirv_cross::SPIRType::Float: scalarSize = 4; break; // 4 bytes for float
            case spirv_cross::SPIRType::Int:   scalarSize = 4; break; // 4 bytes for int
            case spirv_cross::SPIRType::UInt:  scalarSize = 4; break; // 4 bytes for uint
            case spirv_cross::SPIRType::Double: scalarSize = 8; break; // 8 bytes for double
            default:
                ABY_ERR("Unsupported base type for stride calculation!");
                return 0; // Error case
        }

        // Calculate stride for vector or matrix
        uint32_t vectorSize = scalarSize * type.vecsize;       // Vector size = scalar * components
        uint32_t stride = vectorSize;

        // If it's a matrix, account for columns (row-major or column-major padding may also apply)
        if (type.columns > 1) {
            stride *= type.columns; // Matrix stride = vector stride * columns
        }

        return stride;
    }
}

// ShaderCompiler
namespace aby::vk {

    std::vector<std::uint32_t> ShaderCompiler::compile(DeviceManager& devices, const fs::path& path, EShader type) {
        auto cached = cache_dir(path);

        if (fs::exists(cached)) {
            std::vector<std::uint32_t> out;
            std::ifstream in(cached, std::ios::in | std::ios::binary);
            in.seekg(0, std::ios::end);
            auto size = in.tellg();
            in.seekg(0, std::ios::beg);
            out.resize(size / sizeof(uint32_t)); // + (size % sizeof(uint32_t) != 0 ? 1 : 0));
            in.read(reinterpret_cast<char*>(out.data()), size);
            return out;
        }

        if (type == EShader::FROM_EXT) {
            type = get_type_from_ext(path.extension());
        }
    #pragma warning(push, 0)
        shaderc::Compiler compiler;
    #pragma warning(pop)
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetTargetSpirv(shaderc_spirv_version_1_3);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.AddMacroDefinition("GLSL_VERSION", "450");
        options.AddMacroDefinition("MAX_TEXTURE_SLOTS", std::to_string(devices.max_texture_slots()));
        options.AddMacroDefinition("BINDLESS_TEXTURE_BINDING", std::to_string(BINDLESS_TEXTURE_BINDING));
    #ifdef NDEBUG
        options.AddMacroDefinition("NDEBUG");
    #endif
        std::ifstream ifs(path);
        if (!ifs.is_open()) {
            ABY_ERR("Failed to open file: {}", path.string());
        }
        std::stringstream ss;
        ss << ifs.rdbuf();
        ifs.close();
        std::string source(ss.str());

        auto module = compiler.CompileGlslToSpv(source, helper::get_shader_type(type), path.string().c_str(), options);
        std::vector<std::uint32_t> out(module.cbegin(), module.cend());

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            ABY_ERR("{}", module.GetErrorMessage());
            return {};
        }
        std::ofstream ofs(cached, std::ios::out | std::ios::binary);
        if (ofs.is_open()) {
            ofs.write(reinterpret_cast<char*>(out.data()), out.size() * sizeof(std::uint32_t));
            if (!ofs) {
                ABY_ERR("Failed to write to file: {}", cached.string());
                return {};
            }
            ofs.flush();
            ofs.close();
        }
        else {
            ABY_ERR("Failed to open file: {}", cached.string());
            return {};
        }

        ABY_DBG("Compiled glsl shader: {}", path.string());
        return out;
    }

    fs::path ShaderCompiler::cache_dir(const fs::path& file) {
        auto cache_dir = App::bin() / "Cache/Shaders";
        if (!fs::exists(cache_dir)) {
            fs::create_directories(cache_dir);
        }
        return cache_dir / file.filename();
    }

    ShaderDescriptor ShaderCompiler::reflect(const std::vector<std::uint32_t>&binary_data) {
        ShaderDescriptor descriptor;

        spirv_cross::Compiler compiler(binary_data);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        // Uniform buffers
        for (const auto& uniform : resources.uniform_buffers) {
            const auto& type = compiler.get_type(uniform.base_type_id);
            uint32_t set = compiler.get_decoration(uniform.id, spv::DecorationDescriptorSet);
            uint32_t binding = compiler.get_decoration(uniform.id, spv::DecorationBinding);

            if (type.basetype == spirv_cross::SPIRType::Struct) {
                uint32_t buffer_size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));

                // Extract struct members (e.g., `MVP { mat4 model, view, proj; }`)
                for (uint32_t i = 0; i < type.member_types.size(); i++) {
                    spirv_cross::SPIRType member_type = compiler.get_type(type.member_types[i]);
                    uint32_t offset = compiler.get_member_decoration(type.self, i, spv::DecorationOffset);
                    descriptor.uniforms.emplace_back(
                        compiler.get_member_name(type.self, i),
                        set,
                        binding,
                        static_cast<uint32_t>(compiler.get_declared_struct_member_size(type, i))
                    );
                }
            }
            else {
                uint32_t size = static_cast<uint32_t>(compiler.get_declared_struct_size(type));
                descriptor.uniforms.emplace_back(uniform.name, set, binding, size);
            }
        }

        // Storage buffers
        for (const auto& storage : resources.storage_buffers) {
            uint32_t set = compiler.get_decoration(storage.id, spv::DecorationDescriptorSet);
            uint32_t binding = compiler.get_decoration(storage.id, spv::DecorationBinding);

            descriptor.storages.push_back({ storage.name, set, binding });
        }

        // Combined image samplers
        for (const auto& sampler : resources.sampled_images) {
            uint32_t set = compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet);
            uint32_t binding = compiler.get_decoration(sampler.id, spv::DecorationBinding);
            auto type = compiler.get_type(sampler.type_id);
            uint32_t count = 1;
            if (type.self == spv::OpTypeArray) {
                count = type.array.size(); // Set the count to the array length
            }

            descriptor.samplers.push_back({ sampler.name, set, binding, count });
        }

        uint32_t global_offset = 0;
        for (const auto& input : resources.stage_inputs) {
            auto     type     = compiler.get_type(input.base_type_id);
            uint32_t location = compiler.get_decoration(input.id, spv::DecorationLocation);
            uint32_t binding  = compiler.get_decoration(input.id, spv::DecorationBinding);
            uint32_t stride   = helper::get_stride(type);
            uint32_t offset   = global_offset;
            global_offset += stride;

            VkFormat format = VK_FORMAT_UNDEFINED;
            if (type.basetype == spirv_cross::SPIRType::Float) {
                switch (type.vecsize) {
                    case 1: format = VK_FORMAT_R32_SFLOAT; break;
                    case 2: format = VK_FORMAT_R32G32_SFLOAT; break;
                    case 3: format = VK_FORMAT_R32G32B32_SFLOAT; break;
                    case 4: format = VK_FORMAT_R32G32B32A32_SFLOAT; break;
                }
            }
            else if (type.basetype == spirv_cross::SPIRType::Int) {
                switch (type.vecsize) {
                    case 1: format = VK_FORMAT_R32_SINT; break;
                    case 2: format = VK_FORMAT_R32G32_SINT; break;
                    case 3: format = VK_FORMAT_R32G32B32_SINT; break;
                    case 4: format = VK_FORMAT_R32G32B32A32_SINT; break;
                }
            }
            else if (type.basetype == spirv_cross::SPIRType::UInt) {
                switch (type.vecsize) {
                    case 1: format = VK_FORMAT_R32_UINT; break;
                    case 2: format = VK_FORMAT_R32G32_UINT; break;
                    case 3: format = VK_FORMAT_R32G32B32_UINT; break;
                    case 4: format = VK_FORMAT_R32G32B32A32_UINT; break;
                }
            } 
            else {
                ABY_ERR("Unsupported shader input type!");
            }

            descriptor.inputs.emplace_back(location, binding, offset, stride, format);
        }
        return descriptor;
    }

    EShader ShaderCompiler::get_type_from_ext(const fs::path& ext) {
        if (ext == ".vert" || ext == ".vertex") {
            return EShader::VERTEX;
        }
        else if (ext == ".frag" || ext == ".fragment") {
            return EShader::FRAGMENT;
        }
        ABY_ASSERT(false, "Shader Extension '{}' not supported", ext);
        return EShader::MAX_ENUM;
    }

}

// Shader
namespace aby::vk {

    Shader::Shader(DeviceManager& devices, const fs::path& path, EShader type) :
        aby::Shader(
            ShaderCompiler::compile(devices, path, type),
            type == EShader::FROM_EXT ? ShaderCompiler::get_type_from_ext(path.extension()) : type
        ),
        m_Logical(devices.logical()),
        m_Module(VK_NULL_HANDLE),
        m_Layout(VK_NULL_HANDLE),
        m_Descriptor(ShaderCompiler::reflect(m_Data)) 
    {
        // Create shader module
        VkShaderModuleCreateInfo smci = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = m_Data.size() * sizeof(std::uint32_t),
            .pCode = m_Data.data(),
        };
        VK_CHECK(vkCreateShaderModule(m_Logical, &smci, IAllocator::get(), &m_Module));

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkDescriptorBindingFlags> binding_flags;
        std::unordered_set<std::size_t> used_bindings;

        VkShaderStageFlags stageFlag = (m_Type == EShader::VERTEX) ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;

        // Process uniform buffers
        for (const auto& uniform : m_Descriptor.uniforms) {
            if (used_bindings.insert(uniform.binding).second) { // Only insert if unique
                VkDescriptorSetLayoutBinding binding{};
                binding.binding = uniform.binding;
                binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                binding.descriptorCount = 1;
                binding.stageFlags = stageFlag;
                binding.pImmutableSamplers = nullptr;

                bindings.push_back(binding);
                binding_flags.push_back(VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT);
            }
        }

        VkDescriptorSetLayoutBinding binding{};
        binding.binding = BINDLESS_TEXTURE_BINDING;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = MAX_BINDLESS_RESOURCES;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding.pImmutableSamplers = nullptr;
        bindings.push_back(binding);
        binding_flags.push_back(
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT           |
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT |
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
        );
      

        // Descriptor binding flags
        VkDescriptorSetLayoutBindingFlagsCreateInfo bfci{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindingFlags = binding_flags.data(),
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
    }


    Ref<Shader> Shader::create(DeviceManager& devices, const fs::path& path, EShader type) {
        return create_ref<Shader>(devices, path, type);
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
        };
        return ci;
    }

    Shader::operator VkShaderModule() const {
        return m_Module;
    }   


}

// ShaderModule
namespace aby::vk {

    ShaderModule::ShaderModule(vk::Context* ctx, const fs::path& vertex, const fs::path& frag) :
        m_Ctx(ctx),
        m_Layout(VK_NULL_HANDLE),
        m_Vertex(ctx->devices(), vertex, EShader::VERTEX),
        m_Fragment(ctx->devices(), frag, EShader::FRAGMENT),
        m_Pool(VK_NULL_HANDLE),
        m_Descriptors(),
        m_Uniforms(VK_NULL_HANDLE),
        m_UniformMemory(VK_NULL_HANDLE),
        m_Class(m_Vertex.descriptor(), 10000, 0)
    {
        std::vector<VkDescriptorSetLayout> descriptor_set_layouts{
           m_Vertex.layout(),
           m_Fragment.layout()
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size()),
            .pSetLayouts = descriptor_set_layouts.data(),
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr,
        };

        std::vector<VkDescriptorPoolSize> pool_sizes = {
           { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1 }, // Adjust counts based on needs
           { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_BINDLESS_RESOURCES }
        };

        VkDescriptorPoolCreateInfo ci{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 20,
            .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
            .pPoolSizes = pool_sizes.data(),
        };

        auto logical = m_Ctx->devices().logical();

        VK_CHECK(vkCreateDescriptorPool(logical, &ci, IAllocator::get(), &m_Pool));
        
        auto descriptor_set_count = static_cast<std::uint32_t>(descriptor_set_layouts.size());

        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT alloc_count_info{};
        auto max_binding = MAX_BINDLESS_RESOURCES - 1;
        std::vector<std::uint32_t> max_bindings(descriptor_set_count, max_binding);
        alloc_count_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
        alloc_count_info.pNext = nullptr;
        alloc_count_info.descriptorSetCount = max_bindings.size();
        alloc_count_info.pDescriptorCounts = max_bindings.data();


        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext = &alloc_count_info;
        allocInfo.descriptorPool = m_Pool;
        allocInfo.descriptorSetCount = descriptor_set_count;
        allocInfo.pSetLayouts = descriptor_set_layouts.data();

        m_Descriptors.resize(descriptor_set_count, VK_NULL_HANDLE);
        VK_CHECK(vkAllocateDescriptorSets(logical, &allocInfo, m_Descriptors.data()));
        
        VK_CHECK(vkCreatePipelineLayout(logical, &pipelineLayoutInfo, IAllocator::get(), &m_Layout));

       if (m_Vertex.descriptor().uniforms.empty()) {
            create_uniform_buffer(0);
            update_descriptor_set(0, 0);
       }
    }


    Ref<ShaderModule> ShaderModule::create(vk::Context* ctx, const fs::path& vert, const fs::path& frag) {
        return create_ref<ShaderModule>(ctx, vert, frag);
    }

    void ShaderModule::create_uniform_buffer(std::size_t size) {
        if (size == 0) {
            size = 16;
        }

        auto logical = m_Ctx->devices().logical();
        auto physical = m_Ctx->devices().physical();

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_CHECK(vkCreateBuffer(logical, &buffer_info, IAllocator::get(), &m_Uniforms));

        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(logical, m_Uniforms, &mem_requirements);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = helper::find_mem_type(mem_requirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, physical);

        VK_CHECK(vkAllocateMemory(logical, &alloc_info, IAllocator::get(), &m_UniformMemory));
        VK_CHECK(vkBindBufferMemory(logical, m_Uniforms, m_UniformMemory, 0));
    }


    void ShaderModule::destroy() {
        auto logical = m_Ctx->devices().logical();

        if (m_Uniforms != VK_NULL_HANDLE) {
            vkDestroyBuffer(logical, m_Uniforms, IAllocator::get());
            m_Uniforms = VK_NULL_HANDLE;
        }

        if (m_UniformMemory != VK_NULL_HANDLE) {
            vkFreeMemory(logical, m_UniformMemory, IAllocator::get());
            m_UniformMemory = VK_NULL_HANDLE;
        }
        if (m_Layout != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(logical, m_Layout, IAllocator::get());
        }

        if (!m_Descriptors.empty()) {
            for (std::size_t i = 0; i < m_Descriptors.size(); i++) {
                vkFreeDescriptorSets(logical, m_Pool, 1, &m_Descriptors[i]);
            }
            m_Descriptors.clear();
        }
        if (m_Pool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(logical, m_Pool, IAllocator::get());
        }

        m_Vertex.destroy();
        m_Fragment.destroy();
    }

    void ShaderModule::set_uniforms(const void* data, std::size_t bytes, std::uint32_t binding) {
        std::size_t expected_size = 0;
        for (auto& uniform : m_Vertex.descriptor().uniforms) {
            expected_size += uniform.size;
        }
        ABY_ASSERT(bytes == expected_size, "Expected size {}, but got {}", expected_size, bytes);

        if (m_Uniforms == VK_NULL_HANDLE || m_UniformMemory == VK_NULL_HANDLE) {
            create_uniform_buffer(bytes);
        }

        update_uniform_memory(data, bytes);
        update_descriptor_set(binding, bytes);
    }

    void ShaderModule::update_uniform_memory(const void* data, std::size_t bytes) {
        auto logical = m_Ctx->devices().logical();
        void* mapped;
        vkMapMemory(logical, m_UniformMemory, 0, bytes, 0, &mapped);
        memcpy(mapped, data, bytes);
        vkUnmapMemory(logical, m_UniformMemory);
    }

    void ShaderModule::update_descriptor_set(std::uint32_t binding, std::size_t bytes) {
        auto logical = m_Ctx->devices().logical();
        std::vector<VkWriteDescriptorSet> writes;
        std::vector<VkDescriptorImageInfo> img_infos;
        img_infos.resize(m_Ctx->textures().size());

        if (bytes == 0) {
            auto& textures = m_Ctx->textures();
            for (const auto& [handle, texture] : textures) {
                auto tex = std::static_pointer_cast<vk::Texture>(texture);
                img_infos[handle] = VkDescriptorImageInfo{
                     .sampler = tex->sampler(),                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
                     .imageView = tex->view(),
                     .imageLayout = tex->layout(),
                };
                VkWriteDescriptorSet write_image{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = m_Descriptors[1],
                    .dstBinding = BINDLESS_TEXTURE_BINDING,
                    .dstArrayElement = handle,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .pImageInfo = &img_infos[handle],
                    .pBufferInfo = nullptr,
                    .pTexelBufferView = nullptr,
                };
                writes.push_back(write_image);
            }
            vkUpdateDescriptorSets(logical, writes.size(), writes.data(), 0, nullptr);
            return;
        }
        
        VkDescriptorBufferInfo buffer_info{
            .buffer = m_Uniforms,
            .offset = 0,
            .range = bytes,
        };
        VkWriteDescriptorSet write_uniforms{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_Descriptors[0],
            .dstBinding = binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &buffer_info,
            .pTexelBufferView = nullptr
        };
        writes.push_back(write_uniforms);

        for (const auto& [handle, texture] : m_Ctx->textures()) {
            auto tex = std::static_pointer_cast<vk::Texture>(texture);
            img_infos[handle] = VkDescriptorImageInfo{
                 .sampler = tex->sampler(),
                 .imageView = tex->view(),
                 .imageLayout = tex->layout(),
            };
            VkWriteDescriptorSet write_image{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = m_Descriptors[1],
                .dstBinding = BINDLESS_TEXTURE_BINDING,
                .dstArrayElement = handle,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &img_infos[handle],
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr,
            };
            writes.push_back(write_image);
        }

        vkUpdateDescriptorSets(logical, writes.size(), writes.data(), 0, nullptr);
    }

    const Shader& ShaderModule::vert() const {
        return m_Vertex;
    }

    const Shader& ShaderModule::frag() const {
        return m_Fragment;
    }
        
    VkPipelineLayout ShaderModule::layout() const {
        return m_Layout;
    }

    const std::vector<VkDescriptorSet>& ShaderModule::descriptors() const {
        return m_Descriptors;
    }

    std::vector<VkDescriptorSet>& ShaderModule::descriptors() {
        return m_Descriptors;
    }

    std::vector<VkPipelineShaderStageCreateInfo> ShaderModule::stages() const {
        return { m_Vertex.stage(), m_Fragment.stage() };
    }
    
    const VertexClass& ShaderModule::vertex_class() const {
        return m_Class;
    }

}
