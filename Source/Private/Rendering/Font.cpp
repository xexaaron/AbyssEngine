#include "Rendering/Font.h"
#include "Rendering/Context.h"
#include "Core/App.h"
#include <stb_image/stb_image_write.h>
#include <FT/abyft.h>
#include <imgui/imgui.h>

namespace aby {

    Resource Font::create(Context* ctx, const fs::path& path, u32 pt) {
        return ctx->load_thread().add_task(EResource::FONT, [ctx, path, pt]() {
            Timer timer;
            auto font = CreateRefEnabler<Font>::create(ctx, path, ctx->window()->dpi(), pt);
            ABY_LOG("Loaded Font: {}ms", timer.elapsed().milli());
            ABY_LOG("  Name: \"{}\"", font->name());
            ABY_LOG("  Size:  {}pt", font->size());
            return ctx->fonts().add(font);
        });
    }

    Font::Font(Context* ctx, const fs::path& path, const glm::vec2& dpi, u32 pt) :
        m_SizePt(pt),
        m_Data(ft::Library::get().create_font_data(ctx->app()->cache(), ft::FontCfg{
            .pt      = pt, 
            .dpi     = { dpi.x, dpi.y }, 
            .range   = ft::CharRange(32, 128),
            .path    = path,
            .verbose = true,
        }))
    {
        m_Texture            = Texture::create(ctx, m_Data.png);
    }

    Font::~Font() {
    }


    u32 Font::size() const {
        return m_SizePt;
    }

    Resource Font::texture() const {
        return m_Texture;
    }

    std::string_view Font::name() const {
        return m_Data.name;
    }

    const ft::Glyphs& Font::glyphs() const {
        return m_Data.glyphs;
    }
    
    bool Font::is_mono() const {
        return m_Data.is_mono;
    }
    float Font::text_height() const {
        return m_Data.text_height;
    }

    float Font::char_width() const {
        ABY_ASSERT(m_Data.is_mono, "Font is not monospaced! Do not call Font::char_width()");
        return m_Data.glyphs.at(U'a').size.x;
    }


    glm::vec2 Font::measure(const std::string& text) const {
        glm::vec2 size(0, m_Data.text_height);
        if (m_Data.is_mono) {
            size.x = text.length() * m_Data.glyphs.at(U'a').size.x;
            return size;
        }
        for (std::size_t i = 0; i < text.size(); i++) {
            char32_t c = text[i];
            if (auto it = m_Data.glyphs.find(c); it != m_Data.glyphs.end()) {
                const auto& g = it->second;
                size.x += g.advance;
            }
        }
        return size;
    }

   


}
