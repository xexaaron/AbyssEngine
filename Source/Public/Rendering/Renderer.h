#pragma once

#include "Core/Common.h"
#include "Core/Event.h"
#include "Rendering/Context.h"
#include <glm/glm.hpp>
namespace aby {

	struct Vertex {
		Vertex(const glm::vec3& pos, const glm::vec3& col = { 1, 1, 1 }, std::uint32_t texture = {}) :
			pos(pos), col(col), tex(texture) {}
		Vertex(const glm::vec2& pos, const glm::vec3& col = { 1, 1, 1 }, std::uint32_t texture = {}) :
			pos(pos, 0), col(col), tex(texture) {}

		glm::vec3    pos;
		glm::vec3    col;
		float        tex;
	};

	struct Triangle {
		Triangle(const Vertex& v1, const Vertex& v2, const Vertex& v3) : v1(v1), v2(v2), v3(v3) {}
		Triangle(const Triangle& other) : v1(other.v1), v2(other.v2), v3(other.v3) {}
		Triangle(Triangle&& other) noexcept : v1(std::move(other.v1)), v2(std::move(other.v2)), v3(std::move(other.v3)) {}

		Vertex v1, v2, v3;
	};

	struct Quad {
		Quad(const Vertex& v, const glm::vec2& size) : v(v), size(size) {}

		Vertex v;
		glm::vec2 size;
	};

	struct Circle {
		Circle(const Vertex& v, float radius, float points) : v(v), radius(radius), points(points) { }

		float radius;
		float points;
		Vertex v;
	};

    class Renderer abstract {
	public:
        static Ref<Renderer> create(Ref<Context> ctx);
        virtual ~Renderer() = default;

        virtual void destroy() = 0;
		
		virtual void on_event(Event& event) = 0;
		/**
		* @brief Begin 2d/3d rendering.
		*/
		virtual void on_begin(const glm::mat4& view_projection) = 0;
		/**
		* @brief Begin 2d rendering.
		*/
		virtual void on_begin() = 0;
		/**
		* @brief End 2d/3d rendering.
		*/
		virtual void on_end() = 0;
		
		virtual void draw_triangle_3d(const Triangle& triangle) = 0;
		virtual void draw_quad_3d(const Quad& triangle) = 0;
		virtual void draw_circle_3d(const Circle& circle) = 0;

		virtual void draw_triangle_2d(const Triangle& triangle) = 0;
		virtual void draw_quad_2d(const Quad& triangle) = 0;
		virtual void draw_circle_2d(const Circle& circle) = 0;
	protected:
	};

}