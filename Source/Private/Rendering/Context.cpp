#include "Rendering/Context.h"
#include "vk/VkContext.h"
#include "Core/Log.h"


namespace aby {

    Context::Context(const AppInfo& info, Ref<Window> window) :
        m_Backend(info.Backend),
        m_Window(window) 
    {

    }

    Ref<Context> Context::create(const AppInfo& app_info, Ref<Window> window) {
        switch (app_info.Backend) {
            case EBackend::VULKAN: {
                return create_ref<vk::Context>(app_info, window);
            }
            default: 
                ABY_ASSERT(false, "Invalid AppInfo::Backend value");
        }
        return nullptr;
    }


    Ref<Window> Context::window() {
        auto win = m_Window.lock();
        ABY_ASSERT(win, "vk::Context::window() invalid window attmpeting to be returned!");
        return win;
    }

    EBackend Context::backend() const {
        return m_Backend;
    }

    ResourceClass<Shader>& Context::shaders() {
        return m_Shaders;
    }
    const ResourceClass<Shader>& Context::shaders() const {
        return m_Shaders;
    }
    ResourceClass<Texture>& Context::textures() {
        return m_Textures;
    }
    
    const ResourceClass<Texture>& Context::textures() const {
        return m_Textures;
    }

}