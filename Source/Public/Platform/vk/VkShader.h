#pragma once
#include "Core/Common.h"
#include "Rendering/Shader.h"
#include "Platform/vk/VkCommon.h"
#include "Platform/vk/VkDeviceManager.h"
#include "Platform/vk/VkShaderCompiler.h"

namespace aby::vk {

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

}



