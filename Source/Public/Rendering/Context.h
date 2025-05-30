#pragma once
#include "Core/Common.h"
#include "Core/Window.h"
#include "Core/Resource.h"
#include "Core/Thread.h"
#include "Rendering/Font.h"
#include "Rendering/Texture.h"
#include "Rendering/Shader.h"

namespace aby {
    
    class App;

    class Context abstract {
    public:
        static Ref<Context> create(App* app, Window* window);
        virtual ~Context() = default;

        virtual void destroy() = 0; 
        virtual void imgui_init() = 0;
        virtual void imgui_new_frame() = 0;
        virtual void imgui_end_frame() = 0;

        Window*     window();
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
        Context(App* app, Window* window);
    protected:
        App*                   m_App;
        EBackend               m_Backend;
        Window*                m_Window;
        ResourceClass<Shader>  m_Shaders;
        ResourceClass<Texture> m_Textures;
        ResourceClass<Font>    m_Fonts;
        LoadThread             m_LoadThread;
    };

}