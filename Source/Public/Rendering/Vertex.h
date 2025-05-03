#pragma once
#include "Core/Common.h"
#include <glm/glm.hpp>
#include <string>

namespace aby {
    
    struct Vertex {
        Vertex(const glm::vec3& pos, const glm::vec4& col, float texture = 0.f, const glm::vec2& uvs = { 1, 1});
        Vertex(const glm::vec3& pos, const glm::vec4& col, const glm::vec2& texcoord, float texture, const glm::vec2& uvs = { 1, 1 });
        Vertex(const glm::vec3& pos, const glm::vec4& col, const glm::vec3& texinfo, const glm::vec2& uvs = { 1, 1 });
        Vertex(const glm::vec2& pos, const glm::vec4& col, float texture = 0.f, const glm::vec2& uvs = { 1, 1 });
        Vertex(const glm::vec2& pos, const glm::vec4& col, const glm::vec2& texcoord, float texture, const glm::vec2& uvs = { 1, 1 });
        Vertex(const glm::vec2& pos, const glm::vec4& col, const glm::vec3& texinfo, const glm::vec2& uvs = { 1, 1 });

        glm::vec3    pos;
        glm::vec4    col;
        glm::vec3    texinfo; // xy = texcoord, z = texidx
        glm::vec2    uvs;
    };

    struct Triangle {
        Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3);
        Triangle(const Triangle& other);
        Triangle(Triangle&& other) noexcept;

        Vertex v1, v2, v3;
    };

    struct Quad {
        Quad(const glm::vec2& size, const glm::vec2& pos = {}, const glm::vec4& col = { 1, 1, 1, 1 }, float texture = 0.f, const glm::vec2& uvs = { 1, 1 });
        Quad(const glm::vec3& size = {}, const glm::vec3& pos = {}, const glm::vec4& col = { 1, 1, 1, 1 }, float texture = 0.f, const glm::vec2& uvs = { 1, 1 });

        glm::vec3 pos;
        glm::vec4 col;
        glm::vec3 texinfo; // xy = texcoord, z = texidx
        glm::vec2 uvs;
        glm::vec3 size;
    };

    struct Text {
        Text(const std::string& text, const glm::vec2& pos, const glm::vec4& color = { 1, 1, 1, 1 }, float scale = 1.f, u32 font = 0);

        glm::vec3     pos;
        float         scale;
        glm::vec4     color;
        u32           font;
        std::string   text;
        std::string   prefix;
    };

}