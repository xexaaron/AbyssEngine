#pragma once
#include "Platform/vk/VkCommon.h"
#include "Platform/vk/VkBuffer.h"
#include "Platform/vk/VkCmdPool.h"
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
        void set_dbg_name(const std::string& name) override;

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
#ifndef NDEBUG
        std::string     m_DbgName;
#endif
    };

    struct PerTex {
        VkImage         img = VK_NULL_HANDLE;
        VkDeviceMemory  mem = VK_NULL_HANDLE;
        VkImageView     view = VK_NULL_HANDLE;
        VkDescriptorSet set = VK_NULL_HANDLE;
        glm::u32vec2    size = { 0, 0 };
        VkFormat        fmt = VK_FORMAT_UNDEFINED;
        VkImageLayout   layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    };

    class BufferedTexture : public aby::BufferedTexture {
    public:
        BufferedTexture(vk::Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format, std::size_t buffers);
        BufferedTexture(vk::Context* ctx, const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format, std::size_t buffers);
        ~BufferedTexture();

        void write(const glm::u32vec2& size, const std::vector<std::byte>& data) override;
        void write(const glm::u32vec2& size, const void* data) override;
        void set_dbg_name(const std::string& name) override;
        void set_max_buffers(std::size_t frames) override;

        VkImage img();
        VkImageView view();
        VkSampler sampler();
        VkImageLayout layout() const;
        VkDescriptorSet& imgui_descriptor();
        ImTextureID imgui_id() const override;
        glm::u32vec2 size() const override;
        u32 channels() const override;
        ETextureFormat format() const override;
    private:
        void init(const glm::u32vec2& size, ETextureFormat format, const void* data, std::size_t buffers);
        void init(PerTex& buffer);
        void destroy(PerTex& buffer);
        void upload(PerTex& buffer, const void* src_data);
        void create_sampler();
        PerTex& curr();
        const PerTex& curr() const;
    private:
        static void create_imgui_layout(vk::Context* ctx);
    private:
        vk::Context*        m_Ctx     = nullptr;
        Ref<CmdPool>        m_CmdPool = nullptr;
        VkSampler           m_Sampler = VK_NULL_HANDLE;
        std::size_t         m_Idx     = 0;
        std::vector<PerTex> m_Texs    = {};
#ifndef NDEBUG
        std::string         m_DbgName = "";
#endif
        static inline VkDescriptorSetLayout s_ImGuiLayout = VK_NULL_HANDLE;
    };

}
