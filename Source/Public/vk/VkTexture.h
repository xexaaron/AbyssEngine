#pragma once
#include "vk/VkCommon.h"
#include "vk/VkBuffer.h"
#include "Rendering/Texture.h"

namespace aby::vk {
    
    class Context;

    class Texture : public aby::Texture {
    public:
        Texture(vk::Context* ctx); 
        Texture(vk::Context* ctx, const fs::path& path);
        Texture(vk::Context* ctx, const glm::u32vec2& size, const glm::vec4& color);
        Texture(vk::Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels);
        Texture(const Texture& other);
        Texture(Texture&& other) noexcept;
        ~Texture();

        VkImage img();
        VkImageView view();
        VkSampler sampler();
        VkFormat format() const;
        VkImageLayout layout() const;
    protected:
        void init(vk::Context* ctx);
    private:
        VkDevice m_Logical;
        VkFormat m_Format;
        VkImageLayout m_Layout;
        VkImage m_Image;
        VkImageView m_View;
        VkDeviceMemory m_ImageMemory;
        VkSampler m_Sampler;
    };

}
