#pragma once
#include "Core/Common.h"
#include "Core/Window.h"
#include "Core/Resource.h"
#include "Rendering/Texture.h"
#include "Rendering/Shader.h"

namespace aby {
    
    class Context abstract {
    public:
        static Ref<Context> create(const AppInfo& app_info, Ref<Window> window);
        virtual ~Context() = default;

        virtual void destroy() = 0; 

        Ref<Window> window();
        EBackend    backend() const;

        ResourceClass<Shader>&        shaders();
        const ResourceClass<Shader>&  shaders() const;
        ResourceClass<Texture>&       textures();
        const ResourceClass<Texture>& textures() const;
    protected:
        Context(const AppInfo& info, Ref<Window> window);
    protected:
        EBackend m_Backend;
        Weak<Window> m_Window;
        ResourceClass<Shader> m_Shaders;
        ResourceClass<Texture> m_Textures;
    };

}