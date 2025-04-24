#include "Rendering/Font.h"
#include "Rendering/Context.h"
#include "Core/App.h"
#include <freetype/ft2build.h>
#include <freetype/freetype.h>
#include <stb_image/stb_image_write.h>

#define FT_CHECK(err, ...) ABY_ASSERT(err == FT_Error(0), "[freetype] ({}): {}", err, FT_Error_String(err))

namespace ft {

    class Library {
    public:
        static Library& get() {
            static Library instance;
            return instance;
        }

        FT_Face create_face(const std::filesystem::path& path, std::uint32_t pt, const glm::vec2& dpi) {
            FT_Face face = nullptr;
            std::string path_str = path.string();
            FT_CHECK(::FT_New_Face(m_Library, path_str.c_str(), FT_Long(0), &face));
            FT_CHECK(::FT_Set_Char_Size(face, FT_F26Dot6(0), pt << 6u, static_cast<FT_UInt>(dpi.x), static_cast<FT_UInt>(dpi.y)));
            ABY_LOG("Font Faces:  {}", face->num_faces);
            ABY_LOG("Font Glyphs: {}", face->num_glyphs);
            return face;
        }

        void destroy_face(FT_Face face) {
            ::FT_Done_Face(face);
        }

        FontData load_glyph_range(aby::App* app, char32_t start, char32_t end, const std::filesystem::path& path, std::uint32_t pt, const glm::vec2& dpi) {
            auto name       = path.filename().string();
            auto glyph_file = cache_path(app, start, end, name, pt, ".bin");
            auto png_file   = cache_path(app, start, end, name, pt, ".png");
            
            FontData out;
            if (std::filesystem::exists(png_file) && std::filesystem::exists(glyph_file)) {
                out = load_glyph_range_bin(start, end, glyph_file);
            }
            else {
                FT_Face face = create_face(path, pt, dpi);
                out = load_glyph_range_ttf(start, end, face, png_file);
                cache_glyphs(app, start, end, name, pt, out);
                destroy_face(face);
            }
            out.png  = aby::Texture::create(&app->ctx(), png_file);
            out.name = name;
            return out;
        }

        FontData load_glyph_range_ttf(char32_t start, char32_t end, FT_FaceRec_* face, const std::filesystem::path& png_file) {
            constexpr glm::ivec2 ATLAS_SIZE = { 512, 512 };  // Texture Atlas Size (Fixed or dynamically sized as needed)

            std::uint32_t tex_width = ATLAS_SIZE.x;
            std::uint32_t tex_height = ATLAS_SIZE.y;

            std::vector<char> pixels(tex_width * tex_height, 0); // Initialize pixel buffer with 0 (black)

            int pen_x = 0;
            int pen_y = 0;

            float max_ascent = static_cast<float>(face->size->metrics.ascender) / 64.0f;
            float max_descent = static_cast<float>(face->size->metrics.descender) / 64.0f;
            FontData out{
                .glyphs  = {},
                .text_height = (max_ascent - max_descent) * 0.5f,
                .is_mono = static_cast<bool>(face->face_flags & FT_FACE_FLAG_FIXED_WIDTH),
            };

            for (char32_t c = start; c < end; ++c) {
                int pad = 1;
                if (::FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT))
                    continue;  // If it is not a valid character, continue

                auto* glyph = face->glyph;
                auto* bmp = &glyph->bitmap;

                // If the glyph doesn't fit in the current row, move to the next row
                if (pen_x + bmp->width >= tex_width) {
                    pen_x = 0;
                    pen_y += (face->size->metrics.height >> 6) + pad;
                }
                glm::vec2 uv_min = { static_cast<float>(pen_x) / tex_width, static_cast<float>(pen_y) / tex_height };
                glm::vec2 uv_max = { static_cast<float>(pen_x + bmp->width) / tex_width, static_cast<float>(pen_y + bmp->rows) / tex_height };
                glm::vec4 uvs = { uv_min.x, uv_min.y, uv_max.x, uv_max.y };
                out.glyphs[c] = Glyph{
                    .advance = static_cast<std::uint32_t>(glyph->advance.x >> 6u),
                    .offset = pen_y * tex_width + pen_x,
                    .bearing = { glyph->bitmap_left, glyph->bitmap_top },
                    .size = { bmp->width, bmp->rows },
                    .texcoords = {
                        { uvs.x, uvs.y }, // Top-left  (0)
                        { uvs.z, uvs.y }, // Top-right (1)
                        { uvs.z, uvs.w }, // Bottom-right (2)
                        { uvs.x, uvs.w }  // Bottom-left  (3)
                    },
                };

                for (unsigned int row = 0; row < bmp->rows; ++row) {
                    for (unsigned int col = 0; col < bmp->width; ++col) {
                        unsigned int x = pen_x + col;
                        unsigned int y = pen_y + row;
                        pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];  // Copy pixel data
                    }
                }

                pen_x += bmp->width + pad;
            }

            std::vector<unsigned char> png_data(tex_width * tex_height * 4);
            for (unsigned int i = 0; i < tex_width * tex_height; ++i) {
                png_data[i * 4 + 0] = pixels[i];  // Red channel
                png_data[i * 4 + 1] = pixels[i];  // Green channel
                png_data[i * 4 + 2] = pixels[i];  // Blue channel
                png_data[i * 4 + 3] = 0xff;      // Alpha channel (fully opaque)
            }

            std::string png_file_str = png_file.string();
            ::stbi_write_png(png_file_str.c_str(), tex_width, tex_height, 4, png_data.data(), tex_width * 4);
            
            return out;
        }

        // Glyph Cache Format
        // File: ${m_Name}_${character_start_range}_${character_end_range}_${m_SizePt}.bin
        // Layout:
        //      std::size_t glyph_count. (Not always character_end_range - character_start_range)
        //      float       text_height.
        //      bool        is_monospaced.
        //      { 
        //          char32_t    character
        //          Glyph       glyph    
        //      }...
        // 
        //
        FontData load_glyph_range_bin(char32_t start, char32_t end, const std::filesystem::path& cache) {
            aby::Serializer serializer(aby::SerializeOpts{ .file = cache, .mode = aby::ESerializeMode::READ });
            std::size_t glyph_count = 0;
            FontData out;

            serializer.read(glyph_count);
            serializer.read(out.text_height);
            serializer.read(out.is_mono);

            out.glyphs.reserve(glyph_count);
            for (std::size_t i = 0; i < glyph_count; i++) {
                char32_t character;
                Glyph glyph;
                serializer.read(character);
                serializer.read(glyph);
                out.glyphs.emplace(character, glyph);
            }

            return out;
        }

        void cache_glyphs(aby::App* app, char32_t start, char32_t end, const std::string& name, std::uint32_t pt, const FontData& data) {
            std::filesystem::path bin_cache_path = cache_path(app, start, end, name, pt, ".bin");
            aby::Serializer serializer(aby::SerializeOpts{ .file = bin_cache_path, .mode = aby::ESerializeMode::WRITE });
            serializer.write(data.glyphs.size());
            serializer.write(data.text_height);
            serializer.write(data.is_mono);
            for (const auto& [character, glyph] : data.glyphs) {
                serializer.write(character);
                serializer.write(glyph);
            }
            serializer.save();
        }

        std::filesystem::path cache_path(aby::App* app, char32_t start, char32_t end, const std::string& name, std::uint32_t pt, const std::filesystem::path& ext) {
            auto dir = app->cache() / "Fonts";
            if (!std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
            }
            auto path = name + "_" + std::to_string(start) + "_" + std::to_string(end) + "_" +
                std::to_string(pt) + ext.string();

            return dir / path;
        }

        operator ::FT_Library& () {
            return m_Library;
        }

    private:
        Library() {
            FT_CHECK(::FT_Init_FreeType(&m_Library));
        }

        ~Library() {
            FT_CHECK(::FT_Done_FreeType(m_Library));
        }

        ::FT_Library m_Library = nullptr;
    };

}

namespace aby {

    Resource Font::create(Context* ctx, const fs::path& path, std::uint32_t pt) {
        return ctx->load_thread().add_task(EResource::FONT, [ctx, path, pt]() {
            Timer timer;
            auto font = CreateRefEnabler<Font>::create(ctx, path, ctx->window()->dpi(), pt);
            ABY_LOG("Loaded Font: {}ms", timer.elapsed().milli());
            ABY_LOG("  Name: \"{}\"", font->name());
            ABY_LOG("  Size:  {}pt", font->size());
            return ctx->fonts().add(font);
        });
    }

    Font::Font(Context* ctx, const fs::path& path, const glm::vec2& dpi, std::uint32_t pt) :
        m_SizePt(pt),
        m_Data(ft::Library::get().load_glyph_range(ctx->app(), 32, 128, path, pt, dpi))
    {

    }

    Font::~Font() {
    }


    std::uint32_t Font::size() const {
        return m_SizePt;
    }

    Resource Font::texture() const {
        return m_Data.png;
    }

    std::string_view Font::name() const {
        return m_Data.name;
    }

    const Font::Glyphs& Font::glyphs() const {
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
