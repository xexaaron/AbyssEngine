#pragma once
#include "Core/Common.h"
#include "Core/Resource.h"
#include "Rendering/Texture.h"
#include <unordered_map>
#include <fstream>

namespace aby::ft {
    struct Glyph {
        u32 advance;
        u32 offset;
        glm::vec2     bearing;
        glm::vec2     size;
        glm::vec2     texcoords[4];
    };
    struct FontData {
        using Glyphs = std::unordered_map<char32_t, Glyph>;
        Glyphs        glyphs      = {};
        float         text_height = 0.f;
        bool          is_mono     = false;
        aby::Resource png         = {};
        std::string   name        = "";
    };
}

namespace aby {

    class Context;

    class Font {
    public:
        using Glyphs = std::unordered_map<char32_t, ft::Glyph>;
    public:
        static Resource create(Context* ctx, const fs::path& path, u32 pt = 14);
        ~Font();
        
        Resource         texture() const;
        std::string_view name() const;
        u32    size() const;
        const Glyphs&    glyphs() const;
        bool             is_mono() const;
        float            text_height() const;
        float            char_width() const;
        glm::vec2        measure(const std::string& text) const;
    protected:
        Font(Context* ctx, const fs::path& path, const glm::vec2& dpi, u32 pt = 14);
    private:
        u32 m_SizePt;
        ft::FontData  m_Data;
    };


}