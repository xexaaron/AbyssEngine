#include "vk/VkRenderer.h"
#include "vk/VkAllocator.h"
#include "vk/VkTexture.h"
#include "Core/App.h"
#include "Core/Log.h"
#include <numeric>

namespace aby::vk {
	
    Renderer::Renderer(Ref<vk::Context> ctx) :
        m_Ctx(ctx),
        m_Frames{},
        m_Swapchain(ctx->surface(), ctx->devices(), ctx->window(), m_Frames),
        m_2D(ctx, m_Swapchain, { 
            App::bin() / "Shaders/Default2D.vert",
            App::bin() / "Shaders/Default2D.frag" 
            }, 10000),
        m_3D(ctx, m_Swapchain, { 
            App::bin() / "Shaders/Default3D.vert",
            App::bin() / "Shaders/Default3D.frag" 
            }, 10000),
        m_RecycledSemaphores{},
        m_Img(0)
    {
        m_Ctx->window()->register_event(this, &Renderer::on_event);
        std::vector<int32_t> samplers(m_Ctx->devices().max_texture_slots());
        std::iota(samplers.begin(), samplers.end(), 0);
        Resource default_tex = Texture::create(m_Ctx.get(), { 1, 1 }, { 1, 1, 1, 1 });
    }
    
    void Renderer::draw_triangle_3d(const Triangle& triangle) {
        draw_triangle(m_3D, triangle);
    }
   
    void Renderer::draw_triangle_2d(const Triangle& triangle) {
        draw_triangle(m_2D, triangle);
    }

    void Renderer::draw_quad_3d(const Quad& quad) {
        glm::vec3 tl = { quad.v.pos.x, quad.v.pos.y, quad.v.pos.z };
        glm::vec3 tr = { quad.v.pos.x - quad.size.x, quad.v.pos.y, quad.v.pos.z };
        glm::vec3 bl = { quad.v.pos.x, quad.v.pos.y - quad.size.y, quad.v.pos.z };
        glm::vec3 br = { quad.v.pos.x - quad.size.x, quad.v.pos.y - quad.size.y, quad.v.pos.z };
        draw_triangle_3d(Triangle({ tl, quad.v.col }, { tr, quad.v.col }, { br, quad.v.col }));
        draw_triangle_3d(Triangle({ tl, quad.v.col }, { bl, quad.v.col }, { br, quad.v.col }));
    }

    void Renderer::draw_quad_2d(const Quad& quad) {
        glm::vec3 tl = { quad.v.pos.x, quad.v.pos.y, 0 };
        glm::vec3 tr = { quad.v.pos.x - quad.size.x, quad.v.pos.y, 0 };
        glm::vec3 bl = { quad.v.pos.x, quad.v.pos.y - quad.size.y, 0 };
        glm::vec3 br = { quad.v.pos.x - quad.size.x, quad.v.pos.y - quad.size.y, 0 };
        Vertex    v1 = Vertex(tl, quad.v.col, quad.v.tex);
        Vertex    v2 = Vertex(tr, quad.v.col, quad.v.tex);
        Vertex    v3 = Vertex(br, quad.v.col, quad.v.tex);
        Vertex&   v4 = v1;  
        Vertex    v5 = Vertex(bl, quad.v.col, quad.v.tex);
        Vertex&   v6 = v3; 
        draw_triangle_2d(Triangle(v1, v2, v3));
        draw_triangle_2d(Triangle(v4, v5, v6));
    }
    void Renderer::draw_circle_3d(const Circle& circle) {
    }

    void Renderer::draw_circle_2d(const Circle& circle) {
        constexpr float PI = 3.14159265358979323846f;

        // Center vertex (assumes Vertex has a constructor that takes position & color)
        Vertex center(circle.v.pos, circle.v.col);

        // First point on the circle
        float first_x = center.pos.x + std::cos(0) * circle.radius;
        float first_y = center.pos.y + std::sin(0) * circle.radius;
        Vertex first({ first_x, first_y }, circle.v.col);
        Vertex prev = first;

        // Generate and draw triangles
        for (std::size_t i = 1; i <= circle.points; i++) {
            float angle = (i / static_cast<float>(circle.points)) * 2.0f * PI;
            Vertex next(
                { center.pos.x + std::cos(angle) * circle.radius,
                 center.pos.y + std::sin(angle) * circle.radius },
                circle.v.col
            );

            // Create triangle using (center, prev, next)
            Triangle tri(center, prev, next);
            draw_triangle_2d(tri);

            prev = next;  // Move to the next point
        }
    }


    void Renderer::draw_triangle(RenderModule& module, const Triangle& triangle) {
        auto& acc = module.accumulator();
        auto& verts = module.vertices();
        flush_if(module, verts.count() + 3 > verts.size());
        acc = triangle.v1;
        ++acc;
        acc = triangle.v2;
        ++acc;
        acc = triangle.v3;
        ++acc;
    }

    void Renderer::destroy() {
        auto* logical = m_Ctx->devices().logical();

        for (auto semaphore : m_RecycledSemaphores) {
            vkDestroySemaphore(logical, semaphore, IAllocator::get());
        }
        m_Swapchain.destroy(m_Ctx->devices(), m_Frames);
        m_2D.destroy();
        m_3D.destroy();
    }

    void Renderer::on_begin() {
        start_batch(m_2D);
    }

    void Renderer::on_begin(const glm::mat4& view_projection) {
        start_batch(m_2D);
        start_batch(m_3D);
        m_3D.module()->set_uniforms(&view_projection, sizeof(view_projection));
        m_2D.module()->update_descriptor_set(0, 0);
    }

    void Renderer::on_end() {
        VkResult res;
        std::tie(res, m_Img) = acquire_next_img();
        if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
            if (!on_resize(m_Ctx->window()->width(), m_Ctx->window()->height())) {
                ABY_ERR("Resize failed!");
            }
            std::tie(res, m_Img) = acquire_next_img();
        }
        if (res != VK_SUCCESS) {
            vkQueueWaitIdle(m_Ctx->devices().graphics().Queue);
            return;
        }
        render(m_Img);
        res = present_img(m_Img);

        if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            if (!on_resize(m_Ctx->window()->width(), m_Ctx->window()->height())) {
                ABY_ERR("Resize failed!");
            }
        }
        else if (res != VK_SUCCESS) {
            ABY_ERR("Failed to present swapchain image.");
        }

    }

    void Renderer::render(std::uint32_t img) {
        VkCommandBuffer cmd = m_Frames[img].CmdBuffer;
        VkCommandBufferBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
        };
        VK_CHECK(vkBeginCommandBuffer(cmd, &begin_info));

    // Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
        helper::transition_image_layout(
            cmd,
            m_Swapchain.images()[img],
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            0,                                                     // srcAccessMask (no need to wait for previous operations)
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,                // dstAccessMask
            VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,                   // srcStage
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT        // dstStage
        );

        VkClearValue clear_value{
            .color = {{0.5f, 0.5f, 0.5f, 0.5f}}
        };

        VkRenderingAttachmentInfo color_attachment{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = m_Swapchain.views()[img],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clear_value 
        };

        // Begin rendering
        VkRenderingInfo rendering_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {                         
                 .offset = {
                    0, 0
                 },        
                 .extent = {              
                    .width  = m_Ctx->window()->width(),
                    .height = m_Ctx->window()->height()
                    }
                 },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment 
        };
        VkViewport vp{
          .width = static_cast<float>(m_Ctx->window()->width()),
          .height = static_cast<float>(m_Ctx->window()->height()),
          .minDepth = 0.0f,
          .maxDepth = 1.0f
        };
        VkRect2D scissor{
            .extent = {
                .width = m_Ctx->window()->width(),
                .height = m_Ctx->window()->height()
            }
        };

        vkCmdBeginRendering(cmd, &rendering_info);
        
        m_3D.pipeline().bind(cmd);
        vkCmdSetViewport(cmd, 0, 1, &vp);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
        vkCmdSetFrontFace(cmd, VK_FRONT_FACE_CLOCKWISE);
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        flush(m_3D);
       
        m_2D.pipeline().bind(cmd);
        flush(m_2D);

        vkCmdEndRendering(cmd);

        helper::transition_image_layout(
            cmd,
            m_Swapchain.images()[img],
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,                 // srcAccessMask
            0,                                                      // dstAccessMask
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,        // srcStage
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                  // dstStage
        );
        VK_CHECK(vkEndCommandBuffer(cmd));

        if (m_Frames[img].Release == VK_NULL_HANDLE) {
            VkSemaphoreCreateInfo semaphore_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            VK_CHECK(vkCreateSemaphore(m_Ctx->devices().logical(), &semaphore_info, IAllocator::get(), &m_Frames[img].Release));
        }

        VkPipelineStageFlags wait_stage{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT };

        VkSubmitInfo info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_Frames[img].Acquire,
            .pWaitDstStageMask = &wait_stage,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_Frames[img].Release
        };

        VK_CHECK(vkQueueSubmit(m_Ctx->devices().graphics().Queue, 1, &info, m_Frames[img].QueueSubmit));
    }

    void Renderer::start_batch(RenderModule& module) {
        module.accumulator().reset();
    }

    void Renderer::flush(RenderModule& module) {
        auto& acc   = module.accumulator();
        auto& verts = module.vertices();
        auto* cmd   = m_Frames[m_Img].CmdBuffer;
        if (acc.count()) {
            verts.set_data(acc.data(), acc.bytes(), m_Ctx->devices());
            verts.bind(cmd);
            vkCmdDraw(cmd, verts.count(), 1, 0, 0);
        }
    }

    void Renderer::flush_if(RenderModule& module, bool flush) {
        if (flush) {
            this->flush(module);
            start_batch(module);
        }
    }
   
    void Renderer::on_event(Event& event) {
        EventDispatcher dsp(event);
        dsp.bind(this, &Renderer::on_resize);
    }

    bool Renderer::on_resize(WindowResizeEvent& event) {
        return on_resize(event.w(), event.h());
    }

    bool Renderer::on_resize(std::uint32_t w, std::uint32_t h) {
        if (w == 0 || h == 0) {
            return false;
        }

        VkSurfaceCapabilitiesKHR surface_properties;
        VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Ctx->devices().physical(), m_Ctx->surface(), &surface_properties));

        // Only rebuild the swapchain if the dimensions have changed
        if (surface_properties.currentExtent.width  == m_Swapchain.width()  &&
            surface_properties.currentExtent.height == m_Swapchain.height())
        {
            return false;
        }

        vkDeviceWaitIdle(m_Ctx->devices().logical());
        
        recreate_swapchain();
        
        return true;
    }

    std::pair<VkResult, std::uint32_t> Renderer::acquire_next_img() {
        auto* logical = m_Ctx->devices().logical();
        std::uint32_t img = UINT32_MAX;
        
        VkSemaphore acquire_semaphore;
        if (m_RecycledSemaphores.empty()) {
            VkSemaphoreCreateInfo info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            VK_CHECK(vkCreateSemaphore(logical, &info, IAllocator::get(), &acquire_semaphore));
        }
        else {
            acquire_semaphore = m_RecycledSemaphores.back();
            m_RecycledSemaphores.pop_back();
        }
       
        VkResult res = vkAcquireNextImageKHR(logical, m_Swapchain, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, &img);
        if (res != VK_SUCCESS) {
            m_RecycledSemaphores.push_back(acquire_semaphore);
            return std::make_pair(res, UINT32_MAX);
        }

        auto queue_submit = m_Frames[img].QueueSubmit;
        auto cmd_pool = m_Frames[img].CmdPool;

        if (queue_submit != VK_NULL_HANDLE) {
            vkWaitForFences(logical, 1, &queue_submit, true, UINT64_MAX);
            vkResetFences(logical, 1, &queue_submit);
        }

        if (cmd_pool != VK_NULL_HANDLE) {
            vkResetCommandPool(logical, cmd_pool, 0);
        }

        VkSemaphore old_semaphore = m_Frames[img].Acquire;
        if (old_semaphore != VK_NULL_HANDLE) {
            m_RecycledSemaphores.push_back(old_semaphore);
        }

        m_Frames[img].Acquire = acquire_semaphore;

        return std::make_pair(VK_SUCCESS, img);
    }

    VkResult Renderer::present_img(std::uint32_t img) {
        auto swapchain = m_Swapchain.operator VkSwapchainKHR();
        VkPresentInfoKHR present{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_Frames[img].Release,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &img,
        };

        // Present swapchain image
        return vkQueuePresentKHR(m_Ctx->devices().graphics().Queue, &present);
    }

    void Renderer::recreate_swapchain() {
        auto& surface = m_Ctx->surface();
        auto& devices = m_Ctx->devices();
        auto  window  = m_Ctx->window();
        m_Swapchain.create(surface, devices, window, m_Frames);
    }

}

