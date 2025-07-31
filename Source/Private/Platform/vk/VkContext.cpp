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

    /**
    * @brief Derived from: https://github.com/shivang51/bess/blob/main/Bess/src/settings/themes.cpp
    */
    void Context::imgui_setup_style() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // Setting the colors
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.f);
        colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);

        // Accent colors changed to darker olive-green/grey shades
        colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);        // Dark gray for check marks
        colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);       // Dark gray for sliders
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // Slightly lighter gray when active
        colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 0.f);             // Button background
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);    // Button hover state
        colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);     // Button active state
        colors[ImGuiCol_Header] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);           // Dark gray for menu headers
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);    // Slightly lighter on hover
        colors[ImGuiCol_HeaderActive] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);     // Lighter gray when active
        colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);        // Separators in dark gray
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f); // Resize grips in dark gray
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
        colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);        // Tabs background
        colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f); // Darker gray on hover
        colors[ImGuiCol_TabActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.7686f, 0.7686f, 0.7686f, 1.f);
        colors[ImGuiCol_DockingPreview] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f); // Docking preview in gray
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f); // Empty dock background
        // Additional styles
       
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

    void Context::set_dbg_obj_name(VkImage image, const char* name) {
        helper::set_debug_name(m_Devices.logical(), image, name);
    }
   
    void Context::set_dbg_obj_name(VkBuffer buffer, const char* name) {
        helper::set_debug_name(m_Devices.logical(), buffer, name);
    }

    void Context::set_dbg_obj_name(VkImageView view, const char* name) {
        helper::set_debug_name(m_Devices.logical(), view, name);
    }

    void Context::set_dbg_obj_name(VkDescriptorSet set, const char* name) {
        helper::set_debug_name(m_Devices.logical(), set, name);
    }

}