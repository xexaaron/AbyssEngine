#pragma once
#include "Platform/vk/VkDeviceManager.h"
#include "Platform/vk/VkInstance.h"
#include "Platform/vk/VkDebugger.h"
#include "Platform/vk/VkSurface.h"
#include "Core/Window.h"
#include "Rendering/Context.h"


namespace aby::vk {

    class Context : public aby::Context {
    public:
        Context(App* app, Window* window);

        static Ref<Context> create(App* app, Window* window);
        void destroy() override;
        void imgui_init() override;
        void imgui_new_frame() override;
        void imgui_end_frame() override;

        void set_dbg_obj_name(VkImage image, const char* name);
        void set_dbg_obj_name(VkBuffer buffer, const char* name);
        void set_dbg_obj_name(VkImageView view, const char* name);
        void set_dbg_obj_name(VkDescriptorSet set, const char* name);

        Instance&      inst();
        Debugger&      debugger();
        DeviceManager& devices();
        Surface&       surface();
    private:
        void imgui_setup_style();
    private:
        Instance m_Instance;
        Debugger m_Debugger;
        DeviceManager m_Devices;
        Surface m_Surface;
    };

}