#pragma once
#include "Core/Common.h"
#include "Core/Object.h"
#include "Rendering/Camera.h"
#include <glm/glm.hpp>

namespace aby {
    
    class Viewport : public Object {
    public:
        Viewport(const glm::u32vec2& size);
        ~Viewport() = default;

		void on_create(App* app, bool deserialized) override;
		void on_event(App* app, Event& event) override;
		void on_tick(App* app, Time deltatime) override;
		void on_destroy(App* app) override;
		
    protected:
        bool on_resize(WindowResizeEvent& event);
    private:
        glm::u32vec2 m_Size;
        FreeCamera m_Camera;
    };

}
