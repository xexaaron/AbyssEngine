#include "Rendering/Vertex.h"

namespace aby {

    Vertex::Vertex(const glm::vec3& pos, const glm::vec4& col, float texture, const glm::vec2& uvs) :
        pos(pos), col(col), texinfo(0, 0, texture), uvs(uvs) {}

    Vertex::Vertex(const glm::vec3& pos, const glm::vec4& col, const glm::vec2& texcoord, float texture, const glm::vec2& uvs) :
        pos(pos), col(col), texinfo(texcoord, texture), uvs(uvs) {}

    Vertex::Vertex(const glm::vec3& pos, const glm::vec4& col, const glm::vec3& texinfo, const glm::vec2& uvs) :
        pos(pos), col(col), texinfo(texinfo), uvs(uvs) {}

    Vertex::Vertex(const glm::vec2& pos, const glm::vec4& col, float texture, const glm::vec2& uvs) :
        pos(pos, 0), col(col), texinfo(0, 0, texture), uvs(uvs) {}

    Vertex::Vertex(const glm::vec2& pos, const glm::vec4& col, const glm::vec2& texcoord, float texture, const glm::vec2& uvs) :
        pos(pos, 0), col(col), texinfo(texcoord, texture), uvs(uvs) {}

    Vertex::Vertex(const glm::vec2& pos, const glm::vec4& col, const glm::vec3& texinfo, const glm::vec2& uvs) :
        pos(pos, 0), col(col), texinfo(texinfo), uvs(uvs) {}

}

namespace aby {

    Triangle::Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) : 
        v1(v1), v2(v2), v3(v3) {}

    Triangle::Triangle(const Triangle& other) : 
        v1(other.v1), v2(other.v2), v3(other.v3) {}

    Triangle::Triangle(Triangle&& other) noexcept : 
        v1(std::move(other.v1)), v2(std::move(other.v2)), v3(std::move(other.v3)) {}

}

namespace aby {

    Quad::Quad(const glm::vec2& size, const glm::vec2& pos, const glm::vec4& col, float texture, const glm::vec2& uvs) :
        pos(glm::vec3(pos + (size / glm::vec2(2, 2)), 0.f)), col(col), texinfo(0, 0, texture), uvs(uvs), size(size, 0.f) {}

    Quad::Quad(const glm::vec3& size, const glm::vec3& pos, const glm::vec4& col, float texture, const glm::vec2& uvs) :
        pos(pos), col(col), texinfo(0, 0, texture), uvs(uvs), size(size) {}

}

namespace aby {

    Text::Text(const std::string& text, const glm::vec2& pos, const glm::vec4& color, float scale, std::uint32_t font) :
        pos(pos, 0.f), scale(scale), color(color), font(font), text(text) {}

}