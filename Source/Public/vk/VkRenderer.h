#pragma once
#include "vk/VkContext.h"
#include "vk/VkCmdBuff.h"
#include "vk/VkRenderModule.h"
#include "Rendering/Renderer.h"
#include "Rendering/Vertex.h"
#include <glm/glm.hpp>

namespace aby::vk {

	class Renderer : public ::aby::Renderer {
	public:
		Renderer(Ref<vk::Context> ctx);
        void destroy() override;

		void on_begin(const glm::mat4& view_projection) override;
        void on_begin() override;
        void on_end() override;
        void on_event(Event& event) override;

        void draw_text(const Text& text) override;
        void draw_triangle_3d(const Triangle& triangle) override;
        void draw_triangle_2d(const Triangle& triangle) override;
        void draw_quad_3d(const Quad& quad) override;
        void draw_quad_2d(const Quad& quad) override;
    protected: 
        void render(std::uint32_t img);
        void start_batch(RenderModule& module);
        void flush(RenderModule& module, ERenderPrimitive primitive);
        void flush_if(RenderModule& module, bool flush, ERenderPrimitive primitive);
        void draw_triangle(RenderModule& module, const Triangle& triangle);
        void draw_quad(RenderModule& module, const Quad& quad);
    private:
        bool on_resize(WindowResizeEvent& event);
        bool on_resize(std::uint32_t w, std::uint32_t h);
        void recreate_swapchain();
        std::pair<VkResult, std::uint32_t> acquire_next_img();
        VkResult present_img(std::uint32_t img);
    private:
        Ref<vk::Context> m_Ctx;
        std::vector<Frame> m_Frames;
        vk::Swapchain m_Swapchain;
        RenderModule m_2D;
        RenderModule m_3D;
        std::vector<VkSemaphore> m_RecycledSemaphores;
        std::uint32_t m_Img;
    };

}