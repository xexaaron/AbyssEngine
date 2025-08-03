#include "Rendering/Texture.h"
#include "Core/Log.h"
#include "Core/App.h"
#include "Platform/vk/VkTexture.h"
#include "Platform/vk/VkContext.h"
#include <stb_image/stb_image.h>
#include <array>


namespace aby {

    static bool check_format_channels(u32 channels, ETextureFormat format) {
        ABY_ASSERT(channels > 0 && channels <= 4, "Invalid channel range");
        switch (channels) {
            case 1: return format == ETextureFormat::R;
            case 2: return format == ETextureFormat::RG;
            case 3: return format == ETextureFormat::RGB;
            case 4: return format == ETextureFormat::RGBA || format == ETextureFormat::BGRA;
            default: std::unreachable();
        }
    }

    Resource Texture::create(Context* ctx, const fs::path& path) {
        ABY_ASSERT(ctx, "(aby::Context*)ctx is invalid!");
        switch (ctx->backend()) {
            case EBackend::VULKAN: {
                return ctx->load_thread().add_task(EResource::TEXTURE, [ctx = ctx, path = path]() {
                    Timer timer;
                    auto tex = create_ref<vk::Texture>(static_cast<vk::Context*>(ctx), path);
                    auto elapsed = timer.elapsed();
                    ABY_LOG("Loaded Texture: {}ms", elapsed.milli());
                    ABY_LOG("  Path:     {}", path);
                    ABY_LOG("  Size:     ({}, {})", EXPAND_VEC2(tex->size()));
                    ABY_LOG("  Channels: {}", tex->channels());
                    ABY_LOG("  Bytes:    {}", tex->bytes());
                    return ctx->textures().add(tex);
                });
            }
            default:
                ABY_ASSERT(false, "Unsupported ctx backend");
                break;
        }
        return {};
    }

    Resource Texture::create(Context* ctx) {
        ABY_ASSERT(ctx, "(aby::Context*)ctx is invalid!");
        switch (ctx->backend()) {
            case EBackend::VULKAN:
            {
                return ctx->load_thread().add_task(EResource::TEXTURE, [ctx]() {
                    auto tex = create_ref<vk::Texture>(static_cast<vk::Context*>(ctx));
                    return ctx->textures().add(tex);
                });
            }
            default:
                ABY_ASSERT(false, "Unsupported ctx backend");
        }
        return {};
    }

    Resource Texture::create(Context* ctx, const glm::u32vec2& size, const glm::vec4& color) {
        ABY_ASSERT(ctx, "(aby::Context*)ctx is invalid!");
        switch (ctx->backend()) {
            case EBackend::VULKAN:
            {
                return ctx->load_thread().add_task(EResource::TEXTURE, [ctx, size, color]() {
                    Timer timer;
                    auto tex = create_ref<vk::Texture>(static_cast<vk::Context*>(ctx), size, color);
                    auto elapsed = timer.elapsed();
                    ABY_LOG("Loaded Texture: {}ms", elapsed.milli());
                    ABY_LOG("  Color:    ({}, {}, {}, {})", EXPAND_COLOR(color));
                    ABY_LOG("  Size:     ({}, {})", EXPAND_VEC2(tex->size()));
                    ABY_LOG("  Channels: {}", tex->channels());
                    ABY_LOG("  Bytes:    {}", tex->bytes());
                    return ctx->textures().add(tex);
                });
            }
            default:
                ABY_ASSERT(false, "Unsupported ctx backend");
                break;

        }
        return {};
    }
    
    Resource Texture::create(Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format) {
        ABY_ASSERT(ctx, "(aby::Context*)ctx is invalid!");
        switch (ctx->backend()) {
            case EBackend::VULKAN:
            {
                return ctx->load_thread().add_task(EResource::TEXTURE, [ctx, size, data, channels, format]() {
                    Timer timer;
                    auto tex = create_ref<vk::Texture>(static_cast<vk::Context*>(ctx), size, data, channels, format);
                    auto elapsed = timer.elapsed();
                    ABY_LOG("Loaded Texture: {}ms", elapsed.milli());
                    ABY_LOG("  Size:     ({}, {})", EXPAND_VEC2(tex->size()));
                    ABY_LOG("  Channels: {}", tex->channels());
                    ABY_LOG("  Bytes:    {}", tex->bytes());
                    return ctx->textures().add(tex);
                });
            }
            default:
                ABY_ASSERT(false, "Unsupported ctx backend");
                break;
        }
        return {};
    }

    Resource Texture::create(Context* ctx, const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format) {
        ABY_ASSERT(ctx, "(aby::Context*)ctx is invalid!");
        switch (ctx->backend()) {
        case EBackend::VULKAN: {
            return ctx->load_thread().add_task(EResource::TEXTURE, [ctx, size, data, channels, format]() {
                Timer timer;
                auto tex = create_ref<vk::Texture>(static_cast<vk::Context*>(ctx), size, data, channels, format);
                auto elapsed = timer.elapsed();
                ABY_LOG("Loaded Texture: {}ms", elapsed.milli());
                ABY_LOG("  Size:     ({}, {})", EXPAND_VEC2(tex->size()));
                ABY_LOG("  Channels: {}", tex->channels());
                ABY_LOG("  Bytes:    {}", tex->bytes());
                return ctx->textures().add(tex);
            });
        }
        default:
            ABY_ASSERT(false, "Unsupported ctx backend");
            break;
        }
        return {};
    }

    Texture::Texture() :
        m_Size(0, 0),
        m_Channels(0),
        m_AbyFormat(ETextureFormat::RGBA),
        m_State(ETextureState::GOOD)
    { }

    Texture::Texture(const fs::path& path) :
        m_Size(0, 0),
        m_Channels(0),
        m_AbyFormat(ETextureFormat::RGBA),
        m_State(ETextureState::GOOD)

    {
        auto str = path.string();
        const char* file = str.c_str();
        int w, h, c;
        constexpr int LOAD_ALL_CHANNELS = 0;

        unsigned char* data = stbi_load(file, &w, &h, &c, LOAD_ALL_CHANNELS);
        if (!data) {
            ABY_ERR("[stbi_image::stbi_load]: {}", stbi_failure_reason());
            return;  
        }
        auto ptr   = reinterpret_cast<std::byte*>(data);
        m_Size.x   = static_cast<unsigned int>(w);
        m_Size.y   = static_cast<unsigned int>(h);
        m_Channels = static_cast<unsigned int>(c);
        m_Data.assign(ptr, ptr + (w * h * c));
        stbi_image_free(data);

        switch (c) {
            case 1: m_AbyFormat = ETextureFormat::R;    break;
            case 2: m_AbyFormat = ETextureFormat::RG;   break;
            case 3: m_AbyFormat = ETextureFormat::RGB;  break;
            case 4: m_AbyFormat = ETextureFormat::RGBA; break;
        }
    }

    Texture::Texture(const glm::u32vec2& size, const glm::vec4& color) :
        m_Size(size),
        m_Channels(4),
        m_AbyFormat(ETextureFormat::RGBA),
        m_State(ETextureState::GOOD)
    {
        size_t byte_count = m_Size.x * m_Size.y * m_Channels;
        std::array<std::byte, 4> rgba = {
            static_cast<std::byte>(static_cast<uint8_t>(color.r * 255.0f)),
            static_cast<std::byte>(static_cast<uint8_t>(color.g * 255.0f)),
            static_cast<std::byte>(static_cast<uint8_t>(color.b * 255.0f)),
            static_cast<std::byte>(static_cast<uint8_t>(color.a * 255.0f))
        };
        m_Data.resize(byte_count);
        for (size_t i = 0; i < byte_count; i += 4) {
            m_Data[i + 0] = rgba[0];
            m_Data[i + 1] = rgba[1];
            m_Data[i + 2] = rgba[2];
            m_Data[i + 3] = rgba[3];
            //std::memcpy(&m_Data[i], rgba.data(), 4);
        }
    }
    
    Texture::Texture(const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format) :
        m_Size(size),
        m_Channels(channels), 
        m_Data(data),
        m_AbyFormat(format),
        m_State(ETextureState::GOOD)
    {
        ABY_ASSERT(data.size() % channels == 0, "Invalid texture data size");
        ABY_ASSERT(m_Size.x * m_Size.y * channels == data.size(), "Data size does not match square image");
        ABY_ASSERT(check_format_channels(channels, format), "Channel count does not align with texture format");
    }

    Texture::Texture(const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format) :
        m_Size(size), 
        m_Channels(channels),
        m_AbyFormat(format),
        m_State(ETextureState::GOOD)
    {
        ABY_ASSERT(data != nullptr, "Input texture data pointer is null");
        ABY_ASSERT(check_format_channels(channels, format), "Channel count does not align with texture format");
        const std::size_t byte_ct = size.x * size.y * channels;
        m_Data.resize(byte_ct);
        std::memcpy(m_Data.data(), data, byte_ct);
    }

    Texture::Texture(const Texture& other) :
        m_Size(other.m_Size),
        m_Channels(other.m_Channels),
        m_Data(other.m_Data),
        m_AbyFormat(other.m_AbyFormat),
        m_State(other.m_State)
    {

    }

    Texture::Texture(Texture&& other) noexcept :
        m_Size(std::move(other.m_Size)),
        m_Channels(std::move(other.m_Channels)),
        m_Data(std::move(other.m_Data)),
        m_AbyFormat(std::move(other.m_AbyFormat)),
        m_State(std::move(other.m_State))
    {

    }

   
    void Texture::write(const glm::u32vec2& size, const std::vector<std::byte>& data) {
        m_State = m_Size != size ? ETextureState::RECREATE : ETextureState::UPLOAD;
        m_Data  = data;
        m_Size  = size;
    }
    
    void Texture::write(const glm::u32vec2& size, const void* data) {
        std::size_t byte_ct = size.x * size.y * m_Channels;

        m_State = m_Size != size ? ETextureState::RECREATE : ETextureState::UPLOAD;
        m_Size  = size;

        m_Data.resize(byte_ct);
        std::memcpy(m_Data.data(), data, byte_ct);
    }

    const glm::u32vec2& Texture::size() const {
        return m_Size;
    }

    u32 Texture::channels() const {
        return m_Channels;
    }

    ETextureFormat Texture::format() const
    {
        return m_AbyFormat;
    }

    std::size_t Texture::bytes() const {
        return m_Data.size();
    }

    bool Texture::dirty() const {
        return m_State != ETextureState::GOOD;
    }


    std::span<const std::byte> Texture::data() const {
        return std::span(m_Data.cbegin(), m_Data.size());
    }

}

namespace aby {

    Ref<BufferedTexture> BufferedTexture::create(Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format, std::size_t buffers) {
        switch (ctx->backend()) {
            case EBackend::VULKAN: {
                return create_ref<vk::BufferedTexture>(static_cast<vk::Context*>(ctx), size, data, channels, format, buffers);
            }
            default:
                ABY_ASSERT(false, "Not implemented case!");
        }
        return nullptr;
    }

    Ref<BufferedTexture> BufferedTexture::create(Context* ctx, const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format, std::size_t buffers) {
        switch (ctx->backend()) {
            case EBackend::VULKAN: {
                return create_ref<vk::BufferedTexture>(static_cast<vk::Context*>(ctx), size, data, channels, format, buffers);
            }
            default:
                ABY_ASSERT(false, "Not implemented case!");
        }
        return nullptr;
    }

}