#include "Platform/vk/VkTexture.h"
#include "Platform/vk/VkContext.h"
#include "Platform/vk/VkAllocator.h"
#include "Platform/vk/VkRenderer.h"
#include "Core/App.h"
#include "Utility/Profiler.h"

// Texture
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
        m_Ctx(ctx),
        m_Handler(nullptr),
        m_Handle(Resource::null)
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
        m_Ctx(ctx),
        m_Handler(nullptr),
        m_Handle(Resource::null)
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
        m_Ctx(ctx),
        m_Handler(nullptr),
        m_Handle(Resource::null)
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
        m_Ctx(ctx),
        m_Handler(nullptr),
        m_Handle(Resource::null)
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
        m_Ctx(ctx),
        m_Handler(nullptr),
        m_Handle(Resource::null)
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
        m_Ctx(other.m_Ctx),
        m_Handler(other.m_Handler),
        m_Handle(other.m_Handle)
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
        m_Ctx(std::move(other.m_Ctx)),
        m_Handler(std::move(other.m_Handler)),
        m_Handle(std::move(other.m_Handle))
    {
    }

    void Texture::init() {
        auto c = this->channels();
        auto& size = this->size();

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

    void Texture::set_dbg_name(const std::string& name) {
#ifndef NDEBUG
        m_DbgName = name;
        m_Ctx->set_dbg_obj_name(m_Image, m_DbgName.c_str());
        m_Ctx->set_dbg_obj_name(m_View, m_DbgName.c_str());
#endif
    }

    void Texture::destroy() {
        if (m_Sampler)     vkDestroySampler(m_Logical, m_Sampler, IAllocator::get());
        if (m_View)        vkDestroyImageView(m_Logical, m_View, IAllocator::get());
        if (m_Image)       vkDestroyImage(m_Logical, m_Image, IAllocator::get());
        if (m_ImageMemory) vkFreeMemory(m_Logical, m_ImageMemory, IAllocator::get());

        m_Sampler     = VK_NULL_HANDLE;
        m_View        = VK_NULL_HANDLE;
        m_Image       = VK_NULL_HANDLE;
        m_ImageMemory = VK_NULL_HANDLE;
    }
    
    void Texture::sync() {
        switch (m_State) {
            case ETextureState::GOOD:     
                return;
            case ETextureState::RECREATE: {
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
        auto  view    = data();
        auto& dim     = size();
        auto& devices = m_Ctx->devices();

        ABY_ASSERT(!view.empty(), "No data to upload");
        ABY_ASSERT(m_Image != VK_NULL_HANDLE, "Texture image is not initialized");

        vk::Buffer staging(view.data(), view.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, m_Ctx->devices());
        Ref<CmdPool> cmd_pool = devices.create_cmd_pool();
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

        helper::copy_buffer_to_img(cmd, staging, m_Image, dim.x, dim.y);

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
            devices.graphics().Queue
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


// BufferedTexture
namespace aby::vk {

    static VkFormat aby_fmt_to_vk_fmt(ETextureFormat fmt) {
        switch (fmt) {
        case ETextureFormat::R:
            return VK_FORMAT_R8_SRGB;
        case ETextureFormat::RG:
            return VK_FORMAT_R8G8_SRGB;
        case ETextureFormat::RGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case ETextureFormat::RGBA:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case ETextureFormat::BGRA:
            return VK_FORMAT_B8G8R8A8_SRGB;
        default:
            ABY_ASSERT(false, "Case not implemented!");
        }
        return VK_FORMAT_UNDEFINED;
    }

    void BufferedTexture::create_imgui_layout(vk::Context* ctx) {
        if (s_ImGuiLayout != VK_NULL_HANDLE) return;

        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layout_info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &binding,
        };

        auto logical = ctx->devices().logical();
        VK_CHECK(vkCreateDescriptorSetLayout(logical, &layout_info, vk::Allocator::get(), &s_ImGuiLayout));
    }

    BufferedTexture::BufferedTexture(vk::Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format, std::size_t buffers) : 
        m_Ctx(ctx),
        m_CmdPool(ctx->devices().create_cmd_pool())
    {
        init(size, format, data.data(), buffers);
    }

    BufferedTexture::BufferedTexture(vk::Context* ctx, const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format, std::size_t buffers) :
        m_Ctx(ctx),
        m_CmdPool(ctx->devices().create_cmd_pool())
    {
        init(size, format, data, buffers);
    }

    BufferedTexture::~BufferedTexture() {
        for (auto& tex : m_Texs)
            destroy(tex);

        if (m_Sampler) 
            vkDestroySampler(m_Ctx->devices().logical(), m_Sampler, nullptr);
    }

    void BufferedTexture::init(const glm::u32vec2& size, ETextureFormat format, const void* data, std::size_t buffers) {
        std::size_t frames = buffers == 0 ? static_cast<vk::Renderer&>(m_Ctx->app()->renderer()).swapchain().frames_in_flight() : buffers;
        m_Texs.resize(frames);
        create_imgui_layout(m_Ctx);
        create_sampler();
        auto fmt = aby_fmt_to_vk_fmt(format);
        for (auto& tex : m_Texs) {
            tex.size = size;
            tex.fmt = fmt;
            tex.layout = VK_IMAGE_LAYOUT_UNDEFINED;
            init(tex);
            upload(tex, data);
        }
    }

    void BufferedTexture::write(const glm::u32vec2& size, const std::vector<std::byte>& data) {
        write(size, data.data());
    }

    void BufferedTexture::write(const glm::u32vec2& size, const void* data) {
        PROFILE_SCOPE("Write");
        m_Idx = (m_Idx + 1) % m_Texs.size();
        PerTex& target = m_Texs[m_Idx];
        if (target.size != size) {
            PROFILE_SCOPE("Recreate");
            destroy(target);
            target.size = size;
            target.fmt = curr().fmt;
            init(target);
        }
        {
            PROFILE_SCOPE("Upload");
            upload(target, data);
        }
    }


    void BufferedTexture::set_dbg_name(const std::string& name) {
#ifndef NDEBUG
        m_DbgName = name;
#endif
    }

    void BufferedTexture::set_max_buffers(std::size_t frames) {
        ABY_ASSERT(frames > 1 && frames <= MAX_FRAMES_IN_FLIGHT, "max buffers out of range");
        if (m_Texs.size() > frames) {
            auto diff = m_Texs.size() - frames;
            for (std::size_t i = 0; i < diff; i++) {
                destroy(m_Texs.back());
                m_Texs.pop_back();
            }
        }
        else if (m_Texs.size() < frames) {
            auto diff = frames - m_Texs.size();
            int chans = channels();
            std::vector<std::byte> dummy(chans, std::byte{ 0xFF });
            for (std::size_t i = 0; i < diff; i++) {
                PerTex tex = {};
                tex.size = { 1, 1 };
                tex.fmt = m_Texs.empty() ? VK_FORMAT_R8G8B8A8_UNORM : m_Texs.back().fmt;
                tex.layout = VK_IMAGE_LAYOUT_UNDEFINED;

                init(tex);
                upload(tex, dummy.data());
            }
        }
    }


    void BufferedTexture::init(PerTex& buffer) {
        VkDevice device = m_Ctx->devices().logical();

        // Create VkImage
        VkImageCreateInfo image_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = buffer.fmt,
            .extent = { buffer.size.x, buffer.size.y, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        VK_CHECK(vkCreateImage(device, &image_info, nullptr, &buffer.img));
        m_Ctx->set_dbg_obj_name(buffer.img, m_DbgName.c_str());

        // Allocate memory
        VkMemoryRequirements mem_reqs;
        vkGetImageMemoryRequirements(device, buffer.img, &mem_reqs);
        VkMemoryAllocateInfo alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = mem_reqs.size,
            .memoryTypeIndex = vk::helper::find_mem_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Ctx->devices().physical()),
        };
        VK_CHECK(vkAllocateMemory(device, &alloc_info, nullptr, &buffer.mem));
        VK_CHECK(vkBindImageMemory(device, buffer.img, buffer.mem, 0));

        // Create image view
        VkImageViewCreateInfo view_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = buffer.img,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = buffer.fmt,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .levelCount = 1,
                .layerCount = 1,
            },
        };



        VK_CHECK(vkCreateImageView(device, &view_info, nullptr, &buffer.view));

        if (curr().layout == VK_IMAGE_LAYOUT_UNDEFINED) {
            VkCommandBuffer cmd = helper::begin_single_time_commands(device, m_CmdPool->operator const VkCommandPool());

            helper::transition_image_layout(
                cmd,
                curr().img,
                &curr().layout,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_ACCESS_NONE,
                VK_ACCESS_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
            );

            helper::end_single_time_commands(cmd, device, m_CmdPool->operator const VkCommandPool(), m_Ctx->devices().graphics().Queue);
        }

        m_Ctx->set_dbg_obj_name(buffer.view, m_DbgName.c_str());
    }

    void BufferedTexture::destroy(PerTex& buffer) {
        VkDevice device = m_Ctx->devices().logical();
        if (buffer.view) vkDestroyImageView(device, buffer.view, nullptr);
        if (buffer.img)  vkDestroyImage(device, buffer.img, nullptr);
        if (buffer.mem)  vkFreeMemory(device, buffer.mem, nullptr);
    }

    void BufferedTexture::upload(PerTex& buffer, const void* src_data) {
        auto device = m_Ctx->devices().logical();
        const VkDeviceSize size_bytes = buffer.size.x * buffer.size.y * channels();
        vk::Buffer staging(src_data, size_bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, m_Ctx->devices());

        Ref<CmdPool> cmd_pool = m_Ctx->devices().create_cmd_pool();
        VkCommandBuffer cmd = helper::begin_single_time_commands(device, cmd_pool->operator const VkCommandPool());

        helper::transition_image_layout(
            cmd,
            buffer.img,
            &buffer.layout,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_ACCESS_SHADER_READ_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT
        );

        helper::copy_buffer_to_img(cmd, staging, buffer.img, buffer.size.x, buffer.size.y);

        // Transition back to shader-readable
        helper::transition_image_layout(
            cmd,
            buffer.img,
            &buffer.layout,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
        );

        helper::end_single_time_commands(
            cmd,
            device,
            cmd_pool->operator const VkCommandPool(),
            m_Ctx->devices().graphics().Queue
        );

        staging.destroy();

        if (buffer.set == VK_NULL_HANDLE) {
            VkDescriptorSetAllocateInfo alloc_info{
              .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
              .descriptorPool = static_cast<vk::Renderer&>(m_Ctx->app()->renderer()).rm2d().module()->pool(), // or make a dedicated pool
              .descriptorSetCount = 1,
              .pSetLayouts = &s_ImGuiLayout,
            };
            VK_CHECK(vkAllocateDescriptorSets(m_Ctx->devices().logical(), &alloc_info, &buffer.set));
            m_Ctx->set_dbg_obj_name(buffer.set, m_DbgName.c_str());
        }

        VkDescriptorImageInfo img_info{
            .sampler     = m_Sampler,
            .imageView   = buffer.view,
            .imageLayout = buffer.layout,
        };
        
        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = buffer.set,
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &img_info,
        };
        vkUpdateDescriptorSets(m_Ctx->devices().logical(), 1, &write, 0, nullptr);
    }

    void BufferedTexture::create_sampler() {
        VkSamplerCreateInfo info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            .maxAnisotropy = 1.f,
            .maxLod = 1.f,
        };
        VK_CHECK(vkCreateSampler(m_Ctx->devices().logical(), &info, nullptr, &m_Sampler));
    }

    PerTex& BufferedTexture::curr() {
        ABY_ASSERT(m_Idx < m_Texs.size(), "Out of bounds");
        return m_Texs[m_Idx];
    }

    const PerTex& BufferedTexture::curr() const {
        ABY_ASSERT(m_Idx < m_Texs.size(), "Out of bounds");
        return m_Texs[m_Idx];
    }

    VkImage BufferedTexture::img() {
        return curr().img;
    }
    
    VkImageView BufferedTexture::view() {
        return curr().view;
    }
    
    VkSampler BufferedTexture::sampler() {
        return m_Sampler;
    }
    
    VkImageLayout BufferedTexture::layout() const {
        return curr().layout;
    }
    
    VkDescriptorSet& BufferedTexture::imgui_descriptor() {
        return curr().set;
    }
    
    ImTextureID BufferedTexture::imgui_id() const {
        return reinterpret_cast<ImTextureID>(curr().set);
    }
    
    glm::u32vec2 BufferedTexture::size() const {
        return curr().size;
    }
    
    u32 BufferedTexture::channels() const {
        switch (curr().fmt) {
            case VK_FORMAT_R8_SRGB:
                return 1;
            case VK_FORMAT_R8G8_SRGB:
                return 2;
            case VK_FORMAT_R8G8B8_SRGB:
                return 3;
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_SRGB:
                return 4;
            default:
                ABY_ASSERT(false, "Case not implemented!");
        }
        return VK_FORMAT_UNDEFINED;
    }
    
    ETextureFormat BufferedTexture::format() const {
        switch (curr().fmt) {
        case VK_FORMAT_R8_SRGB:
            return ETextureFormat::R;
        case VK_FORMAT_R8G8_SRGB:
            return ETextureFormat::RG;
        case VK_FORMAT_R8G8B8_SRGB:
            return ETextureFormat::RGB;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return ETextureFormat::RGBA;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return ETextureFormat::BGRA;
        default:
            ABY_ASSERT(false, "Case not implemented!");
        }
        return ETextureFormat::NONE;
    }

}