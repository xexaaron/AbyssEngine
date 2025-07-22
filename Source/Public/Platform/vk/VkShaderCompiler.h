#pragma once

#include "Core/Common.h"
#include "Rendering/Shader.h"
#include "Platform/vk/VkCommon.h"
#include "Platform/vk/VkDeviceManager.h"
#include "Platform/vk/VkShaderStructs.h"

#include <map>
#include <filesystem>

namespace aby {
    class App;
}

namespace aby::vk {

    class ShaderCompiler {
    public:
        static std::vector<u32> compile(App* app, DeviceManager& devices, const fs::path& path, EShader type = EShader::FROM_EXT);
        static EShader get_type_from_ext(const fs::path& ext);
        static fs::path cache_dir(App* app, const fs::path& file = "");
        static ShaderDescriptor reflect(const std::vector<u32>& binary_data);
    };
}