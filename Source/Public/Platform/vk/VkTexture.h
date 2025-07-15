#pragma once
#include "Platform/vk/VkCommon.h"
#include "Platform/vk/VkBuffer.h"
#include "Rendering/Texture.h"

namespace aby::vk {
    
    class Context;

    class Texture : public aby::Texture {
    public:
        Texture(vk::Context* ctx); 
        Texture(vk::Context* ctx, const fs::path& path);
        Texture(vk::Context* ctx, const glm::u32vec2& size, const glm::vec4& color);
        Texture(vk::Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format);
        Texture(vk::Context* ctx, const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format);
        Texture(const Texture& other);
        Texture(Texture&& other) noexcept;
        ~Texture();

        void sync() override;

        VkImage img();
        VkImageView view();
        VkSampler sampler();
        VkFormat format() const;
        VkImageLayout layout() const;
        VkDescriptorSet& imgui_descriptor();
        ImTextureID imgui_id() const override;
    protected:
        void init();
        void upload();
        void destroy();
    private:
        VkDevice        m_Logical;
        VkFormat        m_Format;
        VkImageLayout   m_Layout;
        VkImage         m_Image;
        VkImageView     m_View;
        VkDeviceMemory  m_ImageMemory;
        VkSampler       m_Sampler;
        VkDescriptorSet m_ImGuiID;
        vk::Context*    m_Ctx;
    private:
        friend class vk::TextureResourceHandler;
        vk::TextureResourceHandler* m_Handler;
        Resource::Handle            m_Handle;
    };

}
