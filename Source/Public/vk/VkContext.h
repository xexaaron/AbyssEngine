#pragma once
#include "vk/VkDeviceManager.h"
#include "vk/VkInstance.h"
#include "vk/VkDebugger.h"
#include "vk/VkSurface.h"
#include "Core/Window.h"
#include "Rendering/Context.h"

namespace aby::vk {

    class Context : public aby::Context {
    public:
        Context(App* app, Window* window);

        static Ref<Context> create(App* app, Window* window);
        void destroy() override;

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