#include "vk/VkRenderer.h"
#include "vk/VkAllocator.h"
#include "vk/VkTexture.h"
#include "Core/App.h"
#include "Core/Log.h"
#include <numeric>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace aby::vk {
    

    Renderer::Renderer(Ref<vk::Context> ctx) :
        m_Ctx(ctx),
        m_Frames{},
        m_Swapchain(ctx->surface(), ctx->devices(), ctx->window(), m_Frames),
        m_2D(ctx, m_Swapchain, { 
            ctx->app()->bin() / "Shaders/Vertex.glsl",
            ctx->app()->bin() / "Shaders/Fragment.glsl" 
        }),
        m_3D(ctx, m_Swapchain, { 
            ctx->app()->bin() / "Shaders/Vertex.glsl",
            ctx->app()->bin() / "Shaders/Fragment.glsl" 
        }),
        m_RecycledSemaphores{},
        m_Img(0)
    {
        m_Ctx->window()->register_event(this, &Renderer::on_event);
        Resource default_tex = Texture::create(m_Ctx.get(), { 1, 1 }, { 1, 1, 1, 1 });
        (void)default_tex;
    }
    
    void Renderer::draw_text(const Text& text) {
        flush_if(m_2D, m_2D.quads().should_flush(text.text.length()), ERenderPrimitive::QUAD);
        m_2D.draw_text(text);
    }

    void Renderer::draw_triangle_3d(const Triangle& triangle) {
        draw_triangle(m_3D, triangle);
    }
   
    void Renderer::draw_triangle_2d(const Triangle& triangle) {
        draw_triangle(m_2D, triangle);
    }

    void Renderer::draw_quad_3d(const Quad& quad) {
        draw_quad(m_3D, quad);
    }

    void Renderer::draw_quad_2d(const Quad& quad) {
        draw_quad(m_2D, quad);
    }

    void Renderer::draw_triangle(RenderModule& module, const Triangle& triangle) {
        flush_if(module, module.tris().should_flush(), ERenderPrimitive::TRIANGLE);
        module.draw_triangle(triangle);
    }
    
    void Renderer::draw_quad(RenderModule& module, const Quad& quad) {
        flush_if(module, module.quads().should_flush(), ERenderPrimitive::QUAD);
        module.draw_quad(quad);
    }

    void Renderer::start_batch(RenderModule& module) {
        module.reset();
    }

    void Renderer::flush(RenderModule& module, ERenderPrimitive primitive) {
        auto* cmd = m_Frames[m_Img].CmdBuffer;
        module.flush(cmd, m_Ctx->devices());
    }

    void Renderer::flush_if(RenderModule& module, bool flush, ERenderPrimitive primitive) {
        if (flush) {
            this->on_end();
            start_batch(module);
            this->on_begin();
        }
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
        auto viewport_size = m_Swapchain.size();
        glm::mat4 ortho_view_proj = glm::ortho(0.0f, static_cast<float>(viewport_size.x), 0.0f, static_cast<float>(viewport_size.y), -1.0f, 1.0f);
        m_2D.module()->set_uniforms(&ortho_view_proj, sizeof(ortho_view_proj));
    }

    void Renderer::on_begin(const glm::mat4& view_projection) {
        start_batch(m_2D);
        start_batch(m_3D);
        m_3D.set_uniforms(&view_projection, sizeof(view_projection));
        auto viewport_size = m_Swapchain.size(); 
        glm::mat4 ortho_view_proj = glm::ortho(0.0f, static_cast<float>(viewport_size.x), 0.0f, static_cast<float>(viewport_size.y), -1.0f, 1.0f);
        m_2D.set_uniforms(&ortho_view_proj, sizeof(ortho_view_proj));
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
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            .pInheritanceInfo = nullptr
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
            .color = {{0.5f, 0.5f, 0.5f, 0.5f}},
        };

        VkRenderingAttachmentInfo color_attachment{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .pNext = nullptr,
            .imageView = m_Swapchain.views()[img],
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = nullptr,
            .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clear_value 
        };

        // Begin rendering
        VkRenderingInfo rendering_info{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .renderArea = VkRect2D{                         
                .offset = VkOffset2D{
                    .x = 0,
                    .y = 0
                 },        
                .extent = VkExtent2D{              
                    .width  = m_Ctx->window()->width(),
                    .height = m_Ctx->window()->height()
                    }
                },
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attachment,
            .pDepthAttachment = nullptr,
            .pStencilAttachment = nullptr
        };
        VkViewport vp{
            .x = 0,
            .y = 0,
            .width = static_cast<float>(m_Ctx->window()->width()),
            .height = static_cast<float>(m_Ctx->window()->height()),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        VkRect2D scissor{
            .offset = {
                .x = 0,
                .y = 0
            },
            .extent = {
                .width = m_Ctx->window()->width(),
                .height = m_Ctx->window()->height()
            }
        };

        vkCmdBeginRendering(cmd, &rendering_info);
        
        m_3D.pipeline().bind(cmd);
        vkCmdSetViewport(cmd, 0, 1, &vp);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdSetCullMode(cmd, VK_CULL_MODE_BACK_BIT);
        vkCmdSetFrontFace(cmd, VK_FRONT_FACE_CLOCKWISE);
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        flush(m_3D, ERenderPrimitive::ALL);
       
        m_2D.pipeline().bind(cmd);
        vkCmdSetCullMode(cmd, VK_CULL_MODE_NONE);
        vkCmdSetPrimitiveTopology(cmd, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        flush(m_2D, ERenderPrimitive::ALL);

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
            VkSemaphoreCreateInfo semaphore_info{ 
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
            };
            VK_CHECK(vkCreateSemaphore(m_Ctx->devices().logical(), &semaphore_info, IAllocator::get(), &m_Frames[img].Release));
        }

        VkPipelineStageFlags wait_stage{ VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT };

        VkSubmitInfo info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
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
            VkSemaphoreCreateInfo info = { 
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0
            };
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
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_Frames[img].Release,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &img,
            .pResults = nullptr
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

