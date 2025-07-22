#pragma once
#include "Core/Common.h"
#include "Core/Resource.h"
#include <span>
#include <glm/glm.hpp>
#include <imgui/imgui.h>

namespace aby {

    class Context;

    /**
    * The byte color format of the texture.
    */
    enum class ETextureFormat {
        R,
        RG,
        RGB,
        RGBA,
        BGRA
    };

    /**
    * Represents the textures state.
    */
    enum class ETextureState {
        GOOD     = 0, /// Nothing needs to be done texture is valid.
        RECREATE = 1, /// Texture has been written to and resized needs recreation.
        UPLOAD   = 2, /// Texture has been written to and requires gpu upload.
    };

    class Texture {
    public:
        /**
        * Create an empty texture.
        * 
        * @param ctx App context
        */
        static Resource create(Context* ctx);
        /**
        * Create a texture from a path.
        * 
        * @param ctx App context
        * @param path Filepath to texture
        */
        static Resource create(Context* ctx, const fs::path& path);
        /**
        * Create a texture filled with a certain color.
        * 
        * @param ctx   App context
        * @param size  Texture size
        * @param color Fill color 
        */
        static Resource create(Context* ctx, const glm::u32vec2& size, const glm::vec4& color = glm::vec4(1.0f));
        /**
        * Create a texture given a vector of bytes.
        * 
        * @param ctx App context
        * @param size Texture size
        * @param data vector of bytes
        * @param channels Number of channels
        * @param format Texture color format
        */
        static Resource create(Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format = ETextureFormat::RGBA);
        /**
        * Create a texture given a vector of bytes.
        * 
        * @param ctx App context
        * @param size Texture size
        * @param data pointer of bytes
        * @param channels Number of channels
        * @param format Texture color format
        */
        static Resource create(Context* ctx, const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format = ETextureFormat::RGBA);

        virtual ~Texture() = default;
        
        /**
        * Upload data to cpu side marking texture as dirty.
        * 
        * @param size Texture size
        * @param data vector of bytes
        */
        void write(const glm::u32vec2& size, const std::vector<std::byte>& data);
        /**
        * Upload data to cpu side marking texture as dirty.
        * 
        * @param size Texture size
        * @param data pointer of bytes
        */
        void write(const glm::u32vec2& size, const void* data);
        /**
        * Sync the cpu buffer with the gpu buffer marking texture as non dirty when finished.
        */
        virtual void sync() = 0;
        /**
        * Get size of texture (w, h)
        */
        const glm::u32vec2& size() const;
        /**
        * Get the number of channels
        */
        u32 channels() const;
        /**
        * Get the number of bytes the texture occupies
        */    
        u64 bytes() const;
        /**
        * Get the texture format
        */
        ETextureFormat format() const;
        /**
        * Get a non owning view of the texture data
        */
        std::span<const std::byte> data() const;
        /**
        * Check if the texture is dirty, if so sync cpu with gpu
        */
        bool dirty() const;
        /**
        * Get the id used by ImGui::Image* functions
        * @return Vulkan: VkDescriptor
        * @return OpenGL: GLuint
        */
        virtual ImTextureID imgui_id() const = 0;
    protected:
        Texture();
        Texture(const fs::path& path);
        Texture(const glm::u32vec2& size, const glm::vec4& color = glm::vec4(1.0f));
        Texture(const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format = ETextureFormat::RGBA);
        Texture(const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format = ETextureFormat::RGBA);
        Texture(const Texture& other);
        Texture(Texture&& other) noexcept;
    private:
        glm::u32vec2    m_Size;
        u32             m_Channels;
    protected:
        ETextureFormat  m_AbyFormat;
        ETextureState   m_State;
    private:
        std::vector<std::byte> m_Data;
    };

    class BufferedTexture {
    public:
        Ref<BufferedTexture> create(Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format = ETextureFormat::RGBA);
        Ref<BufferedTexture> create(Context* ctx, const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format = ETextureFormat::RGBA);
        virtual ~BufferedTexture() = default;

        virtual void write(const glm::u32vec2& size, const std::vector<std::byte>& data) = 0;
        virtual void write(const glm::u32vec2& size, const void* data) = 0;

        virtual glm::u32vec2 size() const = 0;
        virtual u32 channels() const = 0;
        virtual ETextureFormat format() const = 0; 
        virtual ImTextureID imgui_id() const = 0;
    protected:
        BufferedTexture() = default;
    };

}