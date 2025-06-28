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
#include <imgui/imgui_internal.h>

#include "Platform/imgui/imtheme.h"


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
        imgui_setup_style();
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
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

    void Context::imgui_setup_style() {
        auto theme_dir = app()->cache() / "Themes";
        if (imgui::Theme::exists("Default", theme_dir)) {
            imgui::Theme default_theme("Default", theme_dir);
            default_theme.set_current();
            return;
        }

        imgui::Theme theme("Default");

        ImGuiStyle* style = theme.style();
        ImVec4* colors = style->Colors;

        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.29f, 0.48f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        // Style:Button
        colors[ImGuiCol_Button]        = ImVec4(0.f, 0.f, 0.f, 0.f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(1.f, 1.f, 1.f, 0.2f);
        colors[ImGuiCol_ButtonActive]  = ImVec4(0.f, 0.f, 0.f, 0.f);
        colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
        colors[ImGuiCol_InputTextCursor] = colors[ImGuiCol_Text];
        colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
        colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
        colors[ImGuiCol_TabSelected] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
        colors[ImGuiCol_TabSelectedOverline] = colors[ImGuiCol_HeaderActive];
        colors[ImGuiCol_TabDimmed] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
        colors[ImGuiCol_TabDimmedSelected] = ImLerp(colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);
        colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
        colors[ImGuiCol_DockingPreview] = colors[ImGuiCol_HeaderActive] * ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);   // Prefer using Alpha=1.0 here
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);   // Prefer using Alpha=1.0 here
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextLink] = colors[ImGuiCol_HeaderActive];
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
        colors[ImGuiCol_TreeLines] = colors[ImGuiCol_Border];
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

        theme.set_current();
        theme.save(theme_dir);
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