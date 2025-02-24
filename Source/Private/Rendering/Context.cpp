#include "Rendering/Context.h"
#include "vk/VkContext.h"
#include "Core/Log.h"
#include "Core/App.h"


namespace aby {

    Context::Context(App* app, Window* window) :
        m_App(app),
        m_Backend(app->info().backend),
        m_Window(window),
        m_LoadThread([this](EResource type) -> Resource::Handle {
            switch (type) {
                using enum EResource;
                case SHADER:
                    return this->shaders().size();
                case TEXTURE:
                    return this->textures().size();
                case FONT:
                    return this->fonts().size();
            }
            return 0;
        })
    {

    }

    Ref<Context> Context::create(App* app, Window* window) {
        switch (app->info().backend) {
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

    Window* Context::window() {
        ABY_ASSERT(m_Window, "vk::Context::window() invalid window attmpeting to be returned!");
        return m_Window;
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
    
    LoadThread& Context::load_thread() {
        return m_LoadThread;
    }
    
    const LoadThread& Context::load_thread() const {
        return m_LoadThread;
    }

}