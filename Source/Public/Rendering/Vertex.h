#pragma once
#include <glm/glm.hpp>

namespace aby {
    
    struct Vertex {
        Vertex(const glm::vec3& pos, const glm::vec4& col, float texture = 0.f, const glm::vec2& uvs = { 1, 1}) :
            pos(pos), col(col), texinfo(0, 0, texture), uvs(uvs) {}
        Vertex(const glm::vec3& pos, const glm::vec4& col, const glm::vec2& texcoord, float texture, const glm::vec2& uvs = { 1, 1 }) :
            pos(pos), col(col), texinfo(texcoord, texture), uvs(uvs) {}
        Vertex(const glm::vec3& pos, const glm::vec4& col, const glm::vec3& texinfo, const glm::vec2& uvs = { 1, 1 }) :
            pos(pos), col(col), texinfo(texinfo), uvs(uvs) {}
        Vertex(const glm::vec2& pos, const glm::vec4& col, float texture = 0.f, const glm::vec2& uvs = { 1, 1 }) :
            pos(pos, 0), col(col), texinfo(0, 0, texture), uvs(uvs) {}
        Vertex(const glm::vec2& pos, const glm::vec4& col, const glm::vec2& texcoord, float texture, const glm::vec2& uvs = { 1, 1 }) :
            pos(pos, 0), col(col), texinfo(texcoord, texture), uvs(uvs) {}
        Vertex(const glm::vec2& pos, const glm::vec4& col, const glm::vec3& texinfo, const glm::vec2& uvs = { 1, 1 }) :
            pos(pos, 0), col(col), texinfo(texinfo), uvs(uvs) {}

        glm::vec3    pos;
        glm::vec4    col;
        glm::vec3    texinfo; // xy = texcoord, z = texidx
        glm::vec2    uvs;
    };

    struct Triangle {
        Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) : v1(v1), v2(v2), v3(v3) {}
        Triangle(const Triangle& other) : v1(other.v1), v2(other.v2), v3(other.v3) {}
        Triangle(Triangle&& other) noexcept : v1(std::move(other.v1)), v2(std::move(other.v2)), v3(std::move(other.v3)) {}

        Vertex v1, v2, v3;
    };

    struct Quad {
        Quad(const glm::vec2& size, const glm::vec2& pos, const glm::vec4& col = { 1, 1, 1, 1 }, float texture = 0.f, const glm::vec2& uvs = { 1, 1 }) :
            v(glm::vec3(pos + (size / glm::vec2(2, 2)), 0.f), col, texture, uvs), size(size) {
        }
        Quad(const glm::vec2& size = {}, const glm::vec3& pos = {}, const glm::vec4& col = { 1, 1, 1, 1 }, float texture = 0.f, const glm::vec2& uvs = { 1, 1 }) :
            v(glm::vec3(pos.x + (size.x / 2), pos.y + (size.y / 2), 0.f), col, texture, uvs), size(size) { }

        Vertex v;
        glm::vec2 size;
    };

    struct Text {
        Text(const std::string& text, const glm::vec2& pos, const glm::vec4& color = { 1, 1, 1, 1 }, float scale = 1.f, std::uint32_t font = 0) :
            pos(pos, 0.f), scale(scale), color(color), font(font), text(text) {}

        glm::vec3     pos;
        float         scale;
        glm::vec4     color;
        std::uint32_t font;
        std::string   text;
    };

}