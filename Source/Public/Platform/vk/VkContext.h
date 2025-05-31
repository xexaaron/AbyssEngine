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

        Instance&      inst();
        Debugger&      debugger();
        DeviceManager& devices();
        Surface&       surface();
    private:
        Instance m_Instance;
        Debugger m_Debugger;
        DeviceManager m_Devices;
        Surface m_Surface;
    };

}