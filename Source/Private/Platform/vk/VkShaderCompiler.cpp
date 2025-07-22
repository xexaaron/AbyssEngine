#include "Platform/vk/VkShaderCompiler.h"
#include "Core/App.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv_cross/spirv_reflect.hpp>

#include <fstream>

namespace aby::vk::helper {

    static shaderc_shader_kind get_shader_type(EShader type) {
        switch (type) {
            case EShader::VERTEX:
                return shaderc_shader_kind::shaderc_glsl_vertex_shader;
            case EShader::FRAGMENT:
                return shaderc_shader_kind::shaderc_glsl_fragment_shader;
            default:
                throw std::out_of_range("EShader");
        }
    }

    static uint32_t get_stride(const spirv_cross::SPIRType& type) {
        uint32_t size = 0;

        switch (type.basetype) {
            case spirv_cross::SPIRType::Float:  size = 4; break; 
            case spirv_cross::SPIRType::Int:    size = 4; break; 
            case spirv_cross::SPIRType::UInt:   size = 4; break;
            case spirv_cross::SPIRType::Double: size = 8; break; 
            default:
                ABY_ERR("Unsupported base type for stride calculation!");
                return 0; // Error case
        }

        uint32_t stride = size * type.vecsize;     

        if (type.columns > 1) {
            stride *= type.columns; 
        }

        return stride;
    }
}

namespace aby::vk {

    std::vector<u32> ShaderCompiler::compile(App* app, DeviceManager& devices, const fs::path& path, EShader type) {
        auto cached = cache_dir(app, path);
        if (fs::exists(cached)) {
            std::vector<u32> out;
            std::ifstream in(cached, std::ios::in | std::ios::binary);
            in.seekg(0, std::ios::end);
            auto size = in.tellg();
            in.seekg(0, std::ios::beg);
            out.resize(size / sizeof(uint32_t));
            in.read(reinterpret_cast<char*>(out.data()), size);
            return out;
        }

        if (type == EShader::FROM_EXT) {
            type = get_type_from_ext(path.extension());
        }
        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetTargetSpirv(shaderc_spirv_version_1_3);
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.AddMacroDefinition("GLSL_VERSION", "450");
        options.AddMacroDefinition("MAX_TEXTURE_SLOTS", std::to_string(devices.max_texture_slots()));
        options.AddMacroDefinition("BINDLESS_TEXTURE_BINDING", std::to_string(BINDLESS_TEXTURE_BINDING));
        options.AddMacroDefinition("EXPAND_VEC4(vec)", "vec.r, vec.g, vec.b, vec.a");
        options.AddMacroDefinition("EXPAND_VEC3(vec)", "vec.x, vec.y, vec.z");
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
        std::vector<u32> out(module.cbegin(), module.cend());

        if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            ABY_ERR("{}", module.GetErrorMessage());
            return {};
        }
        std::ofstream ofs(cached, std::ios::out | std::ios::binary);
        if (ofs.is_open()) {
            ofs.write(reinterpret_cast<char*>(out.data()), out.size() * sizeof(u32));
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

    fs::path ShaderCompiler::cache_dir(App* app, const fs::path& file) {
        auto bin = app->bin();
        auto cache_dir = bin / "Cache/Shaders";
        if (!fs::exists(cache_dir)) {
            fs::create_directories(cache_dir);
        }
        return cache_dir / file.filename();
    }

    ShaderDescriptor ShaderCompiler::reflect(const std::vector<u32>&binary_data) {
        ShaderDescriptor descriptor;

        spirv_cross::Compiler compiler(binary_data);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        // Uniform buffers
        for (const auto& uniform : resources.uniform_buffers) {
            const auto& type = compiler.get_type(uniform.base_type_id);
            uint32_t set = compiler.get_decoration(uniform.id, spv::DecorationDescriptorSet);
            uint32_t binding = compiler.get_decoration(uniform.id, spv::DecorationBinding);

            if (type.basetype == spirv_cross::SPIRType::Struct) {
                // Extract struct members (e.g., `MVP { mat4 model, view, proj; }`)
                for (uint32_t i = 0; i < type.member_types.size(); i++) {
                    spirv_cross::SPIRType member_type = compiler.get_type(type.member_types[i]);
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
            descriptor.storages.push_back({
                storage.name,
                compiler.get_decoration(storage.id, spv::DecorationDescriptorSet),
                compiler.get_decoration(storage.id, spv::DecorationBinding)
            });
        }

        // Combined image samplers
        for (const auto& sampler : resources.sampled_images) {
            auto& type = compiler.get_type(sampler.type_id);
            descriptor.samplers.push_back({
                sampler.name,
                compiler.get_decoration(sampler.id, spv::DecorationDescriptorSet),
                compiler.get_decoration(sampler.id, spv::DecorationBinding),
                type.self == spv::OpTypeArray ? static_cast<u32>(type.array.size()) : 1
            });
        }

        uint32_t global_offset = 0;
        for (const auto& input : resources.stage_inputs) {
            auto&     type     = compiler.get_type(input.base_type_id);
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
                    default:
                        std::unreachable();
                }
            }
            else if (type.basetype == spirv_cross::SPIRType::Int) {
                switch (type.vecsize) {
                    case 1: format = VK_FORMAT_R32_SINT; break;
                    case 2: format = VK_FORMAT_R32G32_SINT; break;
                    case 3: format = VK_FORMAT_R32G32B32_SINT; break;
                    case 4: format = VK_FORMAT_R32G32B32A32_SINT; break;
                    default:
                        std::unreachable();
                }
            }
            else if (type.basetype == spirv_cross::SPIRType::UInt) {
                switch (type.vecsize) {
                    case 1: format = VK_FORMAT_R32_UINT; break;
                    case 2: format = VK_FORMAT_R32G32_UINT; break;
                    case 3: format = VK_FORMAT_R32G32B32_UINT; break;
                    case 4: format = VK_FORMAT_R32G32B32A32_UINT; break;
                    default:
                        std::unreachable();
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

