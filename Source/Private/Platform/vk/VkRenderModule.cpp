#include "Platform/vk/VkRenderModule.h"
#include "Utility/TagParser.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <unordered_set>


namespace aby::vk {

    RenderPrimitive::RenderPrimitive(Ref<vk::Context> ctx, const ShaderDescriptor& vertex_descriptor, const PrimitiveDescriptor& primitive_descriptor) :
        m_VertexClass(vertex_descriptor, primitive_descriptor.MaxVertices, 0),
        m_VertexAccumulator(m_VertexClass),
        m_VertexBuffer(m_VertexClass, ctx->devices()),
        m_IndexBuffer(primitive_descriptor.MaxIndices * sizeof(u32), ctx->devices()),
        m_Descriptor(primitive_descriptor),
        m_IndexCount(0)
    {
       
    }



    void RenderPrimitive::destroy() {
        m_VertexBuffer.destroy();
        if (m_Descriptor.IndicesPer != m_Descriptor.VerticesPer) {
            m_IndexBuffer.destroy();
        }
    }

    const vk::PrimitiveDescriptor& RenderPrimitive::descriptor() const {
        return m_Descriptor;
    }
    std::size_t RenderPrimitive::index_count() const {
        return m_IndexCount;
    }

    std::size_t RenderPrimitive::vertex_count() const {
        return m_VertexAccumulator.count();
    }

    void RenderPrimitive::set_index_data(const u32* indices, DeviceManager& manager) {
        ABY_ASSERT(m_Descriptor.IndicesPer != m_Descriptor.VerticesPer, "No index buffer will be used to draw this primitive");
        m_IndexBuffer.set_data(indices, sizeof(u32) * m_Descriptor.MaxIndices, manager);
    }

    bool RenderPrimitive::empty() const {
        return this->vertex_count() == 0;
    }

    bool RenderPrimitive::should_flush() const {
        bool flush = m_VertexAccumulator.count() + m_Descriptor.VerticesPer >= m_VertexAccumulator.capacity();
        return flush;
    }

    bool RenderPrimitive::should_flush(std::size_t requested_primitives) const {
        bool flush = m_VertexAccumulator.count() + (m_Descriptor.VerticesPer * requested_primitives) >= m_VertexAccumulator.capacity();
        return flush;
    }


    void RenderPrimitive::bind(VkCommandBuffer cmd, DeviceManager& manager) {
        m_VertexBuffer.set_data(m_VertexAccumulator.data(), m_VertexAccumulator.bytes(), manager);
        m_VertexBuffer.bind(cmd);
        if (m_Descriptor.IndicesPer != m_Descriptor.VerticesPer) {
            m_IndexBuffer.bind(cmd);
        }
    }
    
    void RenderPrimitive::draw(VkCommandBuffer cmd) {
        if (m_Descriptor.IndicesPer == m_Descriptor.VerticesPer) {
            draw_nonindexed(cmd);
        }
        else {
            draw_indexed(cmd);
        }
    }


    void RenderPrimitive::draw_indexed(VkCommandBuffer cmd) {
        vkCmdDrawIndexed(cmd, static_cast<u32>(m_IndexCount), 1u, 0u, 0u, 0u);
    }

    void RenderPrimitive::draw_nonindexed(VkCommandBuffer cmd) {
        vkCmdDraw(cmd, static_cast<u32>(this->vertex_count()), 1u, 0u, 0u);
    }

    void RenderPrimitive::reset() {
        m_VertexAccumulator.reset();
        m_IndexCount = 0;
    }

    RenderPrimitive& RenderPrimitive::operator++() {
        ++m_VertexAccumulator;
        if (m_VertexAccumulator.count() % m_Descriptor.VerticesPer == 0) {
            m_IndexCount += m_Descriptor.IndicesPer;
        }
        return *this;
    }

}


namespace aby::vk {

    static constexpr glm::mat4 UNIT_MATRIX = glm::mat4(1);

    static constexpr glm::vec2 COORDS[4] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f }
    };
    static constexpr glm::vec4 VERTEX_POSITIONS[4] = {
        { -0.5f, -0.5f, 0.0f, 1.0f },
        {  0.5f, -0.5f, 0.0f, 1.0f },
        {  0.5f,  0.5f, 0.0f, 1.0f },
        { -0.5f,  0.5f, 0.0f, 1.0f }
    };

    static const std::unordered_set<char32_t> TEXT_ESCAPE_CHARACTERS = {
        0x27, // '''
        0x22, // '"'
        0x3f, // '?'
        0x5c, // '\'
        0x07, // '\a'
        0x08, // '\b'
        0x0c, // '\f'
        0x0a, // '\n'
        0x0d, // '\r'
        0x09, // '\t'
        0x0b, // '\v'
    };

    glm::mat4 compute_text_transform(const ft::Glyph& g, const glm::vec2& current_position, float text_scale, float text_size_y) {
        glm::vec3 size = { g.size.x * text_scale, g.size.y * text_scale, 0.f };
        glm::vec3 pos = {
            (current_position.x + g.bearing.x * text_scale) + (size.x / 2),
            (current_position.y + (text_size_y - g.bearing.y) * text_scale) + (size.y / 2),
            0.f
        };
        return glm::translate(UNIT_MATRIX, pos) * glm::scale(UNIT_MATRIX, size);
    }




    RenderModule::RenderModule(Ref<vk::Context> ctx, vk::Swapchain& swapchain, const std::vector<fs::path>& shaders) :
        m_Ctx(ctx.get()),
        m_Module(ShaderModule::create(ctx.get(), shaders[0], shaders[1])),
        m_Pipeline(ctx->window(), ctx->devices(), m_Module, swapchain),
        m_Primitives{
            RenderPrimitive(ctx, m_Module->vertex_descriptor(), PrimitiveDescriptor{
                .MaxVertices = 10000,
                .MaxIndices = 30000, 
                .IndicesPer = 3,
                .VerticesPer = 3
            }), // Triangles
            RenderPrimitive(ctx, m_Module->vertex_descriptor(), PrimitiveDescriptor{
                .MaxVertices = 10000 * 2,
                .MaxIndices = 60000 * 2,
                .IndicesPer = 6,
                .VerticesPer = 4
            }) // Quads
        }
    {
        init();
    }

    RenderModule::RenderModule(Ref<vk::Context> ctx, vk::Swapchain& swapchain, Ref<ShaderModule> module) :
        m_Ctx(ctx.get()),
        m_Module(module),
        m_Pipeline(ctx->window(), ctx->devices(), m_Module, swapchain),
        m_Primitives{
            RenderPrimitive(ctx, m_Module->vertex_descriptor(), PrimitiveDescriptor{
                .MaxVertices = 10000,
                .MaxIndices = 30000,
                .IndicesPer = 3,
                .VerticesPer = 3
            }), // Triangles
            RenderPrimitive(ctx, m_Module->vertex_descriptor(), PrimitiveDescriptor{
                .MaxVertices = 10000 * 2,
                .MaxIndices = 60000 * 2,
                .IndicesPer = 6,
                .VerticesPer = 4
            }) // Quads
        }
    {
        init();
    }

    void RenderModule::init() {
        auto& quad_prim = this->quads();
        auto& prim_desc = quad_prim.descriptor();
#ifdef NDEBUG
        uint32_t* indices = new uint32_t[prim_desc.MaxIndices];
        for (uint32_t i = 0, offset = 0; i < prim_desc.MaxIndices; i += prim_desc.IndicesPer, offset += prim_desc.VerticesPer) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
        }
        quad_prim.set_index_data(indices, m_Ctx->devices());
        delete[] indices;
#else
        std::vector<uint32_t> indices(prim_desc.MaxIndices);
        for (uint32_t i = 0, offset = 0; i < prim_desc.MaxIndices; i += prim_desc.IndicesPer, offset += prim_desc.VerticesPer) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 2;
            indices[i + 4] = offset + 3;
            indices[i + 5] = offset + 0;
        }
        quad_prim.set_index_data(indices.data(), m_Ctx->devices());
#endif
    }

    void RenderModule::destroy() {
        m_Pipeline.destroy();
        for (auto& prim : m_Primitives) {
            prim.destroy();
        }
    }

    void RenderModule::reset() {
        for (auto& prim : m_Primitives) {
            prim.reset();
        }
    }
    
    void RenderModule::flush(VkCommandBuffer cmd, DeviceManager& manager, ERenderPrimitive primitive) {
        if (primitive == ERenderPrimitive::ALL) {
            for (auto& prim : m_Primitives) {
                if (!prim.empty()) {
                    prim.bind(cmd, manager);
                    prim.draw(cmd);
                }
            }
        }
        else {
            auto& prim = m_Primitives[static_cast<std::size_t>(primitive)];
            if (!prim.empty()) {
                prim.bind(cmd, manager);
                prim.draw(cmd);
            }
        }
    }

    void RenderModule::set_uniforms(const void* data, std::size_t bytes, u32 binding) {
        m_Module->set_uniforms(data, bytes, binding);
    }

    void RenderModule::draw_triangle(const Triangle& triangle) {
        auto& acc = this->tris();
        acc = triangle.v1;
        ++acc;
        acc = triangle.v2;
        ++acc;
        acc = triangle.v3;
        ++acc;
    }
    
    void RenderModule::draw_quad(const Quad& quad) {
        auto& acc = this->quads();

        glm::mat4 transform = glm::translate(UNIT_MATRIX, quad.pos) * glm::scale(UNIT_MATRIX, quad.size);

        for (std::size_t i = 0; i < std::size(VERTEX_POSITIONS); i++) {
            glm::vec3 pos(transform * VERTEX_POSITIONS[i]);
            glm::vec3 texinfo(COORDS[i], quad.texinfo.z);
            Vertex v(pos, quad.col, texinfo);
            acc = v;
            ++acc;
        }
    }

    void RenderModule::draw_cube(const Quad& quad) {
        auto& acc = this->quads();

        glm::vec3 half_size = quad.size * 0.5f;


        struct FaceTransform {
            glm::vec3 translation;
            glm::vec3 rotation_axis;
            float rotation_angle; // in radians
        };

        const FaceTransform faces[6] = {
            // Front face (+Z)
            { {0, 0, half_size.z}, {0, 0, 0}, 0 },
            // Back face (-Z)
            { {0, 0, -half_size.z}, {0, 1, 0}, glm::pi<float>() },
            // Left face (-X)
            { {-half_size.x, 0, 0}, {0, 1, 0}, -glm::half_pi<float>() },
            // Right face (+X)
            { {half_size.x, 0, 0}, {0, 1, 0}, glm::half_pi<float>() },
            // Top face (+Y)
            { {0, half_size.y, 0}, {1, 0, 0}, -glm::half_pi<float>() },
            // Bottom face (-Y)
            { {0, -half_size.y, 0}, {1, 0, 0}, glm::half_pi<float>() },
        };

        for (const auto& face : faces) {
            glm::mat4 face_transform = glm::translate(UNIT_MATRIX, quad.pos);
            face_transform *= glm::translate(UNIT_MATRIX, face.translation);
            if (face.rotation_angle != 0.0f)
                face_transform *= glm::rotate(UNIT_MATRIX, face.rotation_angle, face.rotation_axis);
            face_transform *= glm::scale(UNIT_MATRIX, { quad.size.x, quad.size.y, 1.0f });

            for (std::size_t i = 0; i < std::size(VERTEX_POSITIONS); i++) {
                glm::vec3 pos(face_transform * VERTEX_POSITIONS[i]);
                glm::vec3 texinfo(COORDS[i], quad.texinfo.z);
                Vertex v(pos, quad.col, texinfo);
                acc = v;
                ++acc;
            }
        }
    }

    void RenderModule::draw_text(const Text& text) {
        Ref<Font>    font_obj = m_Ctx->fonts().at({ EResource::FONT, text.font });
        float        texture = static_cast<float>(font_obj->texture().handle());
        Ref<Texture> tex_obj = m_Ctx->textures().at(font_obj->texture());
        auto&        acc = this->quads();
        const auto&  glyphs = font_obj->glyphs();

        glm::vec2   text_size        = font_obj->measure(text.text) * text.scale;
        glm::vec3   current_position = { text.pos.x, text.pos.y, 0.f }; 
        glm::vec4   color            = text.color;
        std::string stripped_text    = text.prefix + text.text;
        auto        text_decors      = util::parse_and_strip_tags(stripped_text);

        std::size_t cursor = 0;

        for (auto& decor : text_decors) {
            if (decor.type != util::ETextDecor::HIGHLIGHT)
                continue;
            
            if (!font_obj->is_mono())
                throw std::runtime_error("TODO: Implement ETextDecoration::HIGHLIGHT for non mono fonts");

            auto underline_start = glm::vec2{ 
                current_position.x + (font_obj->char_width() * decor.range.start),
                current_position.y - 2.f
            };

            auto underline_end = glm::vec2{
                current_position.x + (font_obj->char_width() * (decor.range.end + 1)),
                current_position.y
            };
            Quad quad({ (underline_end - underline_start).x, text_size.y + 4.f }, underline_start, { 0.1, 0.1, 1.0, 1.f });
            draw_quad(quad);
        }

        for (char32_t c : stripped_text) {
            // Skip escape characters
            if (TEXT_ESCAPE_CHARACTERS.contains(c)) {
                switch (c) {
                case U'\t': {
                    current_position.x += (glyphs.at(U' ').advance * text.scale * 4);
                    break;
                }
                }
                continue;
            }
            auto it = glyphs.find(c);
            if (it == glyphs.end()) {
                continue;
            }
            const auto& glyph = it->second;
            glm::mat4 transform = compute_text_transform(glyph, current_position, text.scale, text_size.y);
            for (std::size_t i = 0; i < 4; i++) {
                glm::vec3 position(transform * VERTEX_POSITIONS[i]);
                glm::vec3 texinfo(glyph.texcoords[i].x, glyph.texcoords[i].y, texture);
                acc = Vertex(position, color, texinfo);
                ++acc;
            }

            for (auto& decor : text_decors) {
                if (cursor >= decor.range.start && cursor <= decor.range.end) {
                    switch (decor.type) {
                    case util::ETextDecor::UNDERLINE: {
                            auto decor_it = glyphs.find('_');
                            if (decor_it == glyphs.end()) {
                                IF_DBG(ABY_WARN("Font Glyph for character '{:#x}' not found", (int32_t)c), ;);
                                continue;
                            }
                            const auto& decor_glyph = decor_it->second;
                            glm::mat4 decor_transform = compute_text_transform(decor_glyph, { current_position.x, current_position.y }, text.scale, text_size.y);
                            for (std::size_t i = 0; i < 4; i++) {
                                glm::vec3 position(decor_transform * VERTEX_POSITIONS[i]);
                                glm::vec3 texinfo(decor_glyph.texcoords[i].x, decor_glyph.texcoords[i].y, texture);
                                acc = Vertex(position, color, texinfo);
                                ++acc;
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }

            current_position.x += glyph.advance * text.scale;
            cursor++;
        }
    }

    RenderPrimitive& RenderModule::quads() {
        std::size_t idx = static_cast<std::size_t>(ERenderPrimitive::QUAD);
        return m_Primitives[idx];
    }
    RenderPrimitive& RenderModule::tris() {
        std::size_t idx = static_cast<std::size_t>(ERenderPrimitive::TRIANGLE);
        return m_Primitives[idx];
    }

    Ref<ShaderModule> RenderModule::module() const {
        return m_Module;
    }

    vk::Pipeline& RenderModule::pipeline() {
        return m_Pipeline;
    }
  



}