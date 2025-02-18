#pragma once
#include "Core/Common.h"
#include "Core/Resource.h"
#include <span>
#include <glm/glm.hpp>

namespace aby {

    class Context;

    class Texture {
    public:
        static Resource create(Context* ctx);
        static Resource create(Context* ctx, const fs::path& path);
        static Resource create(Context* ctx, const glm::u32vec2& size, const glm::vec4& color);
        static Resource create(Context* ctx, const glm::u32vec2& size, const std::vector<std::byte>& data, std::uint32_t channels);

        virtual ~Texture() = default;
        
        const glm::u32vec2& size() const;
        std::uint32_t channels() const;
        std::uint64_t bytes() const;
        std::span<const std::byte> data() const;
    protected:
        Texture();
        Texture(const fs::path& path);
        Texture(const glm::u32vec2& size, const glm::vec4& color = glm::vec4(1.0f));
        Texture(const glm::u32vec2& size, const std::vector<std::byte>& data, std::uint32_t channels);
        Texture(const Texture& other);
        Texture(Texture&& other) noexcept;
    private:
        glm::u32vec2 m_Size;
        std::uint32_t m_Channels;
        std::vector<std::byte> m_Data;
    };

}