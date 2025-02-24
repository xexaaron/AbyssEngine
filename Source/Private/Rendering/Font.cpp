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
            FT_CHECK(::FT_New_Face(m_Library, path_str.c_str(), 0, &face));
            FT_CHECK(::FT_Set_Char_Size(face, 0, pt << 6, dpi.x, dpi.y));
            
            ABY_LOG("Font Faces:  {}", face->num_faces);
            ABY_LOG("Font Glyphs: {}", face->num_glyphs);

            return face;
        }

        void destroy_face(FT_Face face) {
            ::FT_Done_Face(face);
        }

        std::unordered_map<char32_t, aby::Font::Glyph> load_glyph_range(char32_t start, char32_t end, const std::filesystem::path& path, const std::string& name, std::uint32_t pt, const glm::vec2& dpi, const std::filesystem::path& cache, float& text_height) {
            std::unordered_map<char32_t, aby::Font::Glyph> out;
            if (std::filesystem::exists(cache)) {
                auto glyph_file = cache_path(start, end, name, pt, ".bin");
                if (!std::filesystem::exists(glyph_file)) {
                    FT_Face face = create_face(path, pt, dpi);
                    float max_ascent = static_cast<float>(face->size->metrics.ascender) / 64.0f;
                    float max_descent = static_cast<float>(face->size->metrics.descender) / 64.0f;
                    text_height = max_ascent - max_descent;  // Descent is negative in FreeType
                    text_height *= 0.5f;
                    out = load_glyph_range_ttf(start, end, face, cache);
                    cache_glyphs(start, end, name, pt, out, text_height);
                    destroy_face(face);
                }
                else {
                    out = load_glyph_range_bin(start, end, glyph_file, text_height);
                }
            }
            else {
                FT_Face face = create_face(path, pt, dpi);
                float max_ascent = static_cast<float>(face->size->metrics.ascender) / 64.0f;
                float max_descent = static_cast<float>(face->size->metrics.descender) / 64.0f;
                text_height = max_ascent - max_descent;  // Descent is negative in FreeType
                text_height *= 0.5f;
                out = load_glyph_range_ttf(start, end, face, cache);
                cache_glyphs(start, end, name, pt, out, text_height);
                destroy_face(face);
            }
            return out;
        }

        std::unordered_map<char32_t, aby::Font::Glyph> load_glyph_range_ttf(char32_t start, char32_t end, FT_FaceRec_* face, const std::filesystem::path& cache) {
            constexpr glm::ivec2 ATLAS_SIZE = { 512, 512 };  // Texture Atlas Size (Fixed or dynamically sized as needed)

            int max_glyphs = end - start;
            std::uint32_t tex_width = ATLAS_SIZE.x;
            std::uint32_t tex_height = ATLAS_SIZE.y;

            std::vector<char> pixels(tex_width * tex_height, 0); // Initialize pixel buffer with 0 (black)
            std::uint32_t offset = 0;

            int pen_x = 0;
            int pen_y = 0;
            int pad = 1;

            std::unordered_map<char32_t, aby::Font::Glyph> out;

            for (char32_t c = start; c < end; ++c) {
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
                out[c] = aby::Font::Glyph{
                    .advance = static_cast<std::uint32_t>(glyph->advance.x >> 6),
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

                for (int row = 0; row < bmp->rows; ++row) {
                    for (int col = 0; col < bmp->width; ++col) {
                        int x = pen_x + col;
                        int y = pen_y + row;
                        pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];  // Copy pixel data
                    }
                }

                pen_x += bmp->width + pad;
            }

            std::vector<unsigned char> png_data(tex_width * tex_height * 4);
            for (int i = 0; i < tex_width * tex_height; ++i) {
                png_data[i * 4 + 0] = pixels[i];  // Red channel
                png_data[i * 4 + 1] = pixels[i];  // Green channel
                png_data[i * 4 + 2] = pixels[i];  // Blue channel
                png_data[i * 4 + 3] = 0xff;      // Alpha channel (fully opaque)
            }

            std::string cache_path_str = cache.string();
            ::stbi_write_png(cache_path_str.c_str(), tex_width, tex_height, 4, png_data.data(), tex_width * 4);
            
            return out;
        }


        // Glyph Cache Format
        // File: ${m_Name}_${character_start_range}_${character_end_range}_${m_SizePt}.bin
        // Layout:
        //      std::size_t glyph_count. (Not always character_end_range - character_start_range)
        //      { 
        //          char32_t    character
        //          Glyph       glyph    
        //      }...
        // 
        //
        std::unordered_map<char32_t, aby::Font::Glyph> load_glyph_range_bin(char32_t start, char32_t end, const std::filesystem::path& cache, float& text_height) {
            aby::Serializer serializer(aby::SerializeOpts{ .file = cache, .mode = aby::ESerializeMode::READ });
            std::size_t glyph_count = 0;
            serializer.read(glyph_count);
            serializer.read(text_height);
            std::unordered_map<char32_t, aby::Font::Glyph> out;
            out.reserve(glyph_count);
            for (std::size_t i = 0; i < glyph_count; i++) {
                char32_t character;
                aby::Font::Glyph glyph;
                serializer.read(character);
                serializer.read(glyph);
                out.emplace(character, glyph);
            }
            return out;
        }

        std::filesystem::path cache_path(char32_t start, char32_t end, const std::string& name, std::uint32_t pt, const std::filesystem::path& ext) {
            auto dir = aby::App::cache() / "Fonts";
            if (!std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
            }
            auto path = name + "_" + std::to_string(start) + "_" + std::to_string(end) + "_" +
                std::to_string(pt) + ext.string();

            return dir / path;
        }

        void cache_glyphs(char32_t start, char32_t end, const std::string& name, std::uint32_t pt, const std::unordered_map<char32_t, aby::Font::Glyph>& glyphs, float text_height) {
            std::filesystem::path bin_cache_path = cache_path(start, end, name, pt, ".bin");
            aby::Serializer serializer(aby::SerializeOpts{ .file = bin_cache_path, .mode = aby::ESerializeMode::WRITE });
            serializer.write(glyphs.size());
            serializer.write(text_height);
            for (const auto& [character, glyph] : glyphs) {
                serializer.write(character);
                serializer.write(glyph);
            }
            serializer.save();
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
        m_Texture{},
        m_Name(path.filename().string()),
        m_Glyphs{},
        m_TextHeight(0)
    {
        std::pair<char32_t, char32_t> ascii{ 32, 128 };
        auto png  = ft::Library::get().cache_path(ascii.first, ascii.second, m_Name, pt, ".png");
        m_Glyphs  = ft::Library::get().load_glyph_range(ascii.first, ascii.second, path, m_Name, pt, dpi, png, m_TextHeight);
        m_Texture = Texture::create(ctx, png);
    }

    Font::~Font() {
    }


    float Font::size() const {
        return m_SizePt;
    }

    Resource Font::texture() const {
        return m_Texture;
    }

    std::string Font::name() const {
        return m_Name;
    }

    const Font::Glyphs& Font::glyphs() const {
        return m_Glyphs;
    }
    
    glm::vec2 Font::measure(const std::string& text) const {
        glm::vec2 size(0, m_TextHeight);
        for (std::size_t i = 0; i < text.size(); i++) {
            char32_t c = text[i];
            if (auto it = m_Glyphs.find(c); it != m_Glyphs.end()) {
                const auto& g = it->second;
                size.x += g.advance;
                // size.y = std::max(size.y, g.size.y);
            }
        }
        return size;
    }

    float Font::text_height() const {
        return m_TextHeight;
    }


}
