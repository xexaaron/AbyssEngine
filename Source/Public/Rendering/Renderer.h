#pragma once

#include "Core/Common.h"
#include "Core/Event.h"
#include "Rendering/Context.h"
#include "Rendering/Vertex.h"

namespace aby {

    class Renderer abstract {
	public:
        static Ref<Renderer> create(Ref<Context> ctx);
        virtual ~Renderer() = default;

        virtual void destroy() = 0;
		
		virtual void on_event(Event& event) = 0;
		virtual void on_begin(const glm::mat4& view_projection) = 0;
		virtual void on_begin() = 0;
		virtual void on_end() = 0;
		
		virtual void draw_triangle(const Triangle& triangle) = 0;
		virtual void draw_quad(const Quad& quad) = 0;
		virtual void draw_cube(const Quad& quad) = 0;
		virtual void draw_text(const Text& text) = 0;
	};

}