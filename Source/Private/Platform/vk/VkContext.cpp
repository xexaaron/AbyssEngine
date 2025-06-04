#include "Core/App.h"
#include "Core/Log.h"
#include "Platform/vk/VkContext.h"
#include "Platform/vk/VkRenderer.h"
#include "Platform/vk/VkAllocator.h"

#include <vector>

#ifdef _WIN32
    #include <Windows.h>
    #include "vulkan/vulkan_win32.h"
    #define VK_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    #include <glfw/glfw3.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
    #include "vulkan/vulkan_xlib.h"
    #define GLFW_EXPOSE_NATIVE_X11
    #include <glfw/glfw3native.h>
    #define VK_PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_XLIB_SURFACE_EXTENSION_NAME
    #include <glfw/glfw3.h>
#else
    #error "Unsupported Platform"
#endif

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

namespace aby::vk {
    void check_vk_result(VkResult err) {
        VK_CHECK(err);
    }
}

namespace aby::vk {

    Context::Context(App* app, Window* window) :
        aby::Context(app, window) 
    {
        std::vector<const char*> instance_extensions = {
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_PLATFORM_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        };
        instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        std::vector<const char*> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        std::vector<const char*> validation_layers = {
            "VK_LAYER_KHRONOS_validation"
        };
        
        m_Instance.create(app->info(), instance_extensions, validation_layers);
        m_Debugger.create(m_Instance);
        m_Surface.create(m_Instance, window);
        m_Devices.create(m_Instance, m_Surface, device_extensions);
    }

    Ref<Context> Context::create(App* app, Window* window) {
        return create_ref<Context>(app, window);
    }

    void Context::destroy() {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        m_Shaders.clear();
        m_Textures.clear();
        m_Devices.destroy();
        m_Debugger.destroy();
        m_Surface.destroy();
        m_Instance.destroy();
    }

    void Context::imgui_init() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
        ImGui_ImplGlfw_InitForVulkan(m_Window->glfw(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.ApiVersion         = VK_API_VERSION_1_3;
        init_info.Allocator          = vk::Allocator::get();
        init_info.Instance           = m_Instance;
        init_info.Device             = m_Devices.logical();
        init_info.PhysicalDevice     = m_Devices.physical();
        init_info.QueueFamily        = m_Devices.graphics().FamilyIdx;
        init_info.Queue              = m_Devices.graphics().Queue;
        init_info.DescriptorPool     = static_cast<vk::Renderer&>(m_App->renderer()).rm2d().module()->pool();
        init_info.DescriptorPoolSize = 0;
        init_info.PipelineCache      = {};
        init_info.RenderPass         = nullptr;
        init_info.Subpass            = 0;
        init_info.MinImageCount      = 2; // ?
        init_info.ImageCount         = vk::MAX_FRAMES_IN_FLIGHT; // ?
        init_info.MSAASamples        = VK_SAMPLE_COUNT_1_BIT;
        init_info.CheckVkResultFn    = &check_vk_result;
        init_info.UseDynamicRendering = true;
        init_info.PipelineRenderingCreateInfo = static_cast<vk::Renderer&>(m_App->renderer()).rm2d().pipeline().create_info();
        ImGui_ImplVulkan_Init(&init_info);

        auto font = app()->bin() / "Fonts" / "JetBrainsMonoNerdFontMono-Regular.ttf";
        io.Fonts->AddFontFromFileTTF(font.string().c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
    }

    void Context::imgui_new_frame() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Context::imgui_end_frame() {
        ImGui::Render();
        ImGui::EndFrame();
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }

    Instance& Context::inst() {
        return m_Instance;
    }

    Debugger& Context::debugger() {
        return m_Debugger;
    }

    DeviceManager& Context::devices() {
        return m_Devices;
    }

    Surface& Context::surface() {
        return m_Surface;
    }
}