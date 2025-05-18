#pragma once
#include "Core/Common.h"
#include "Core/Resource.h"
#include "Rendering/Texture.h"

#include <FT/abyft.h>

#include <unordered_map>
#include <fstream>



namespace aby {

    class Context;

    class Font {
    public:
        static Resource create(Context* ctx, const fs::path& path, u32 pt = 14);
        ~Font();
        
        Resource          texture() const;
        std::string_view  name() const;
        u32               size() const;
        const ft::Glyphs& glyphs() const;
        bool              is_mono() const;
        float             text_height() const;
        float             char_width() const;
        glm::vec2         measure(const std::string& text) const;
    protected:
        Font(Context* ctx, const fs::path& path, const glm::vec2& dpi, u32 pt = 14);
    private:
        u32 m_SizePt;
        ft::FontData  m_Data;
        Resource m_Texture;
    };


}