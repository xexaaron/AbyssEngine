#include "Rendering/Context.h"
#include "vk/VkContext.h"
#include "Core/Log.h"
#include "Core/App.h"


namespace aby {

    Context::Context(App* app, Ref<Window> window) :
        m_App(app),
        m_Backend(app->info().Backend),
        m_Window(window)
    {

    }

    Ref<Context> Context::create(App* app, Ref<Window> window) {
        switch (app->info().Backend) {
            case EBackend::VULKAN: {
                return create_ref<vk::Context>(app, window);
            }
            default: 
                ABY_ASSERT(false, "Invalid AppInfo::Backend value");
        }
        return nullptr;
    }

    App* Context::app() {
        return m_App;
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

    ResourceClass<Font>& Context::fonts() {
        return m_Fonts;
    }
    const ResourceClass<Font>& Context::fonts() const {
        return m_Fonts;
    }

}