#pragma once
#include "Core/Common.h"
#include "Core/Resource.h"
#include <span>
#include <glm/glm.hpp>
#include <imgui/imgui.h>

namespace aby {

    class Context;

    enum class ETextureFormat {
        R,
        RG,
        RGB,
        RGBA,
        BGRA
    };

    enum class ETextureState {
        GOOD     = 0, // Nothing needs to be done texture is valid.
        RECREATE = 1, // Texture has been written to and resized needs recreation.
        UPLOAD   = 2, // Texture has been written to and requires gpu upload.
    };

    class Texture {
    public:
        static Resource create(Context* ctx);
        static Resource create(Context* ctx, const fs::path& path);
        static Resource create(Context* ctx, const glm::u32vec2& size, const glm::vec4& color = glm::vec4(1.0f));
        static Resource create(Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, u32 channels, ETextureFormat format = ETextureFormat::RGBA);
        static Resource create(Context* ctx, const glm::u32vec2& size, const void* data, u32 channels, ETextureFormat format = ETextureFormat::RGBA);

        virtual ~Texture() = default;
        
        /**
        * @brief Upload data to cpu side marking texture as dirty.
        */
        void write(const glm::u32vec2& size, const std::vector<std::byte>& data);
        /**
        * @brief Upload data to cpu side marking texture as dirty.
        */
        void write(const glm::u32vec2& size, const void* data);
        /**
        * @brief Sync the cpu buffer with the gpu buffer marking texture as non dirty when finished.
        */
        virtual void sync() = 0;

        const glm::u32vec2& size() const;
        u32 channels() const;
        u64 bytes() const;
        std::span<const std::byte> data() const;
        /**
        * @brief Check if the texture is dirty, if so sync cpu with gpu.
        */
        bool dirty() const;
        /**
        * @brief Get the id used by ImGui::Image* functions
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

}