#pragma once
#include <glm/glm.hpp>

namespace aby {
    
    struct Vertex {
        Vertex(const glm::vec3& pos, const glm::vec3& col, float texture = 0.f) :
            pos(pos), col(col), texinfo(0, 0, texture) {}
        Vertex(const glm::vec3& pos, const glm::vec3& col, const glm::vec2& texcoord, float texture) :
            pos(pos), col(col), texinfo(texcoord, texture) {}
        Vertex(const glm::vec3& pos, const glm::vec3& col, const glm::vec3& texinfo) :
            pos(pos), col(col), texinfo(texinfo) {}
        Vertex(const glm::vec2& pos, const glm::vec3& col, float texture = 0.f) :
            pos(pos, 0), col(col), texinfo(0, 0, texture) {}
        Vertex(const glm::vec2& pos, const glm::vec3& col, const glm::vec2& texcoord, float texture) :
            pos(pos, 0), col(col), texinfo(texcoord, texture) {}
        Vertex(const glm::vec2& pos, const glm::vec3& col, const glm::vec3& texinfo) :
            pos(pos, 0), col(col), texinfo(texinfo) {}

        glm::vec3    pos;
        glm::vec3    col;
        glm::vec3    texinfo; // xy = texcoord, z = texidx
    };

    struct Triangle {
        Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) : v1(v1), v2(v2), v3(v3) {}
        Triangle(const Triangle& other) : v1(other.v1), v2(other.v2), v3(other.v3) {}
        Triangle(Triangle&& other) noexcept : v1(std::move(other.v1)), v2(std::move(other.v2)), v3(std::move(other.v3)) {}

        Vertex v1, v2, v3;
    };

    struct Quad {
        Quad(const glm::vec2& size, const glm::vec3& pos, const glm::vec3& col, float texture = 0.f) : 
            v(pos, col, texture), size(size) { }

        Vertex v;
        glm::vec2 size;
    };

}