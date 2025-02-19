#pragma once
#include "Core/Common.h"
#include "Core/Window.h"
#include "Core/Resource.h"
#include "Rendering/Font.h"
#include "Rendering/Texture.h"
#include "Rendering/Shader.h"

namespace aby {
    
    class App;

    class Context abstract {
    public:
        static Ref<Context> create(App* app, Ref<Window> window);
        virtual ~Context() = default;

        virtual void destroy() = 0; 

        Ref<Window> window();
        App*        app();
        EBackend    backend() const;

        ResourceClass<Shader>&        shaders();
        const ResourceClass<Shader>&  shaders() const;
        ResourceClass<Texture>&       textures();
        const ResourceClass<Texture>& textures() const;
        ResourceClass<Font>&          fonts();
        const ResourceClass<Font>&    fonts() const;
        LoadThread&                   load_thread();
        const LoadThread&             load_thread() const;
    protected:
        Context(App* app, Ref<Window> window);
    protected:
        EBackend m_Backend;
        Weak<Window> m_Window;
        App* m_App;
        ResourceClass<Shader>  m_Shaders;
        ResourceClass<Texture> m_Textures;
        ResourceClass<Font>    m_Fonts;
        LoadThread             m_LoadThread;
    };

}