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
        void draw_triangle(const Triangle& triangle) override;
        void draw_quad(const Quad& quad) override;
        void draw_cube(const Quad& quad) override;
    protected: 
        void render(u32 img);
        void start_batch(RenderModule& module);
        void flush(RenderModule& module, ERenderPrimitive primitive);
        void flush_if(RenderModule& module, bool flush, ERenderPrimitive primitive);
    private:
        bool on_resize(WindowResizeEvent& event);
        bool on_resize(u32 w, u32 h);
        void recreate_swapchain();
        std::pair<VkResult, u32> acquire_next_img();
        VkResult present_img(u32 img);
    private:
        Ref<vk::Context> m_Ctx;
        std::vector<Frame> m_Frames;
        vk::Swapchain m_Swapchain;
        RenderModule m_2D;
        RenderModule m_3D;
        std::vector<VkSemaphore> m_RecycledSemaphores;
        u32 m_Img;
    };

}