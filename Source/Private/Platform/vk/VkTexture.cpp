#include "Platform/vk/VkTexture.h"
#include "Platform/vk/VkContext.h"
#include "Platform/vk/VkAllocator.h"

namespace aby::vk {
    
    Texture::Texture(vk::Context* ctx) :
        aby::Texture(),
        m_Logical(ctx->devices().logical()),
        m_Format(VK_FORMAT_UNDEFINED),
        m_Layout(VK_IMAGE_LAYOUT_UNDEFINED),
        m_Image(VK_NULL_HANDLE),
        m_View(VK_NULL_HANDLE),
        m_ImageMemory(VK_NULL_HANDLE),
        m_Sampler(VK_NULL_HANDLE),
        m_ImGuiID(VK_NULL_HANDLE),
        m_Ctx(ctx)
    {
    }

    Texture::Texture(vk::Context* ctx, const glm::u32vec2& size, const glm::vec4& color) :
        aby::Texture(size, color),
        m_Logical(ctx->devices().logical()),
        m_Format(VK_FORMAT_R8G8B8A8_UINT),
        m_Layout(VK_IMAGE_LAYOUT_UNDEFINED),
        m_Image(VK_NULL_HANDLE),
        m_View(VK_NULL_HANDLE),
        m_ImageMemory(VK_NULL_HANDLE),
        m_Sampler(VK_NULL_HANDLE),
        m_ImGuiID(VK_NULL_HANDLE),
        m_Ctx(ctx)
    {
        init();
    }
    
    Texture::Texture(vk::Context* ctx, const fs::path& path) :
        aby::Texture(path),
        m_Logical(ctx->devices().logical()),
        m_Format(VK_FORMAT_UNDEFINED),
        m_Layout(VK_IMAGE_LAYOUT_UNDEFINED),
        m_Image(VK_NULL_HANDLE),
        m_View(VK_NULL_HANDLE),
        m_ImageMemory(VK_NULL_HANDLE),
        m_Sampler(VK_NULL_HANDLE),
        m_ImGuiID(VK_NULL_HANDLE),
        m_Ctx(ctx)
    {
        init();
    }

    Texture::Texture(vk::Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format) :
        aby::Texture(size, data, channels, format),
        m_Logical(ctx->devices().logical()),
        m_Format(m_AbyFormat == ETextureFormat::BGRA ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_UNDEFINED),
        m_Layout(VK_IMAGE_LAYOUT_UNDEFINED),
        m_Image(VK_NULL_HANDLE),
        m_View(VK_NULL_HANDLE),
        m_ImageMemory(VK_NULL_HANDLE),
        m_Sampler(VK_NULL_HANDLE),
        m_ImGuiID(VK_NULL_HANDLE),
        m_Ctx(ctx)
    {
        init();
    }

    Texture::Texture(vk::Context* ctx, const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format) :
        aby::Texture(size, data, channels, format),
        m_Logical(ctx->devices().logical()),
        m_Format(m_AbyFormat == ETextureFormat::BGRA ? VK_FORMAT_B8G8R8A8_SRGB : VK_FORMAT_UNDEFINED),
        m_Layout(VK_IMAGE_LAYOUT_UNDEFINED),
        m_Image(VK_NULL_HANDLE),
        m_View(VK_NULL_HANDLE),
        m_ImageMemory(VK_NULL_HANDLE),
        m_Sampler(VK_NULL_HANDLE),
        m_ImGuiID(VK_NULL_HANDLE),
        m_Ctx(ctx)
    {
        init();
    }



    Texture::Texture(const Texture& other) :
        aby::Texture(other),
        m_Logical(other.m_Logical),
        m_Format(other.m_Format),
        m_Layout(VK_IMAGE_LAYOUT_UNDEFINED),
        m_Image(other.m_Image),
        m_View(other.m_View),
        m_ImageMemory(other.m_ImageMemory),
        m_Sampler(other.m_Sampler),
        m_ImGuiID(other.m_ImGuiID),
        m_Ctx(other.m_Ctx)
    {

    }
     
    Texture::Texture(Texture&& other) noexcept :
        aby::Texture(std::move(other)),
        m_Logical(std::move(other.m_Logical)),
        m_Format(other.m_Format),
        m_Layout(other.m_Layout),
        m_Image(std::move(other.m_Image)),
        m_View(std::move(other.m_View)),
        m_ImageMemory(std::move(other.m_ImageMemory)),
        m_Sampler(std::move(other.m_Sampler)),
        m_ImGuiID(std::move(other.m_ImGuiID)),
        m_Ctx(std::move(other.m_Ctx))
    {
    }

    void Texture::init() {
        auto c = this->channels();
        auto size = this->size();

        const std::byte* p = this->data().data();
        const void* vp = static_cast<const void*>(p);
        ABY_ASSERT(p, "Data is not valid");
        ABY_ASSERT(vp, "Data is not valid");

        vk::Buffer staging(vp, this->bytes(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, m_Ctx->devices());
        Ref<CmdPool> cmd_pool = m_Ctx->devices().create_cmd_pool();

        if (m_Format == VK_FORMAT_UNDEFINED) {
            switch (c) {
                case 4:
                    m_Format = VK_FORMAT_R8G8B8A8_SRGB;
                    break;
                case 3:
                    m_Format = VK_FORMAT_R8G8B8_SRGB;
                    break;
                case 2:
                    m_Format = VK_FORMAT_R8G8_SRGB;
                    break;
                case 1:
                    m_Format = VK_FORMAT_R8_SRGB;
                    break;
                default:
                    break;
            }
        }

        helper::create_img(
            size.x, size.y, m_Format,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            m_Image, m_ImageMemory,
            m_Ctx->devices().logical(),
            m_Ctx->devices().physical()
        );
        VkCommandBuffer cmd = helper::begin_single_time_commands(m_Logical, cmd_pool.get()->operator const VkCommandPool());

        helper::transition_image_layout(
            cmd,
            m_Image,
            &m_Layout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            0,                                 // No access required for VK_IMAGE_LAYOUT_UNDEFINED
            VK_ACCESS_TRANSFER_WRITE_BIT,      // We are going to write to it using a buffer copy
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // Before anything happens
            VK_PIPELINE_STAGE_TRANSFER_BIT     // Ensure the transfer completes before use
        );

        helper::copy_buffer_to_img(cmd, staging, m_Image, size.x, size.y);

        helper::transition_image_layout(
            cmd,
            m_Image,
            &m_Layout,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,    // Ensure transfer completes
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT // The fragment shader will sample this image
        );

        helper::end_single_time_commands(
            cmd,
            m_Logical,
            cmd_pool.get()->operator const VkCommandPool(),
            m_Ctx->devices().graphics().Queue
        );

        staging.destroy();
        helper::create_img_view(m_Logical, m_Image, m_Format, m_View);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE; // If you have mipmaps, adjust accordingly

        VK_CHECK(vkCreateSampler(m_Logical, &samplerInfo, IAllocator::get(), &m_Sampler));
        cmd_pool->destroy(m_Logical);
    }

    Texture::~Texture() {
        destroy();
    }
    
    void Texture::destroy() {
        if (m_Sampler)     vkDestroySampler(m_Logical, m_Sampler, IAllocator::get());
        if (m_View)        vkDestroyImageView(m_Logical, m_View, IAllocator::get());
        if (m_Image)       vkDestroyImage(m_Logical, m_Image, IAllocator::get());
        if (m_ImageMemory) vkFreeMemory(m_Logical, m_ImageMemory, IAllocator::get());

        m_Sampler = VK_NULL_HANDLE;
        m_View = VK_NULL_HANDLE;
        m_Image = VK_NULL_HANDLE;
        m_ImageMemory = VK_NULL_HANDLE;
    }
    
    void Texture::sync() {
        switch (m_State) {
            case ETextureState::GOOD:     
                return;
            case ETextureState::RECREATE: {
                ABY_ASSERT(m_ShaderModule, "Shader module not set? How the fuck did we get here? Goddamnit Aaron you broke it again.");
                destroy();
                init();
            } break;
            case ETextureState::UPLOAD:   
                upload();
                break;
        }
        m_State = ETextureState::GOOD;
    }

    void Texture::upload() {
        ABY_ASSERT(!data().empty(), "No data to upload");
        ABY_ASSERT(m_Image != VK_NULL_HANDLE, "Texture image is not initialized");

        vk::Buffer staging(data().data(), data().size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, m_Ctx->devices());
        Ref<CmdPool> cmd_pool = m_Ctx->devices().create_cmd_pool();

        VkCommandBuffer cmd = helper::begin_single_time_commands(m_Logical, cmd_pool->operator const VkCommandPool());

        // Optional: transition to transfer dst if layout is not already optimal
        helper::transition_image_layout(
            cmd,
            m_Image,
            &m_Layout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_SHADER_READ_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT
        );

        helper::copy_buffer_to_img(cmd, staging, m_Image, size().x, size().y);

        // Transition back to shader-readable
        helper::transition_image_layout(
            cmd,
            m_Image,
            &m_Layout,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        );

        helper::end_single_time_commands(
            cmd,
            m_Logical,
            cmd_pool->operator const VkCommandPool(),
            m_Ctx->devices().graphics().Queue
        );

        staging.destroy();
        cmd_pool->destroy(m_Logical);
    }


    VkImage Texture::img() {
        return m_Image;
    }

    VkImageView Texture::view() {
        return m_View;
    }
    
    VkSampler Texture::sampler() {
        return m_Sampler;
    }
    
    VkFormat Texture::format() const {
        return m_Format;
    }

    VkImageLayout Texture::layout() const {
        return m_Layout;
    }

    VkDescriptorSet& Texture::imgui_descriptor() {
        return m_ImGuiID;
    }

    ImTextureID Texture::imgui_id() const {
        return reinterpret_cast<ImTextureID>(m_ImGuiID);
    }

}