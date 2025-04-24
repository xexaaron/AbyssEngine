#pragma once
#include "Rendering/UI/Panel.h"
#include "Rendering/Camera.h"

namespace aby::ui {
    
    class Canvas : public Panel {
    public:
        static Ref<Canvas> create(const Style& style);
        Canvas(const Style& style);
        virtual ~Canvas() = default;
        
        void on_tick(App* app, Time deltatime) override;
        void on_create(App* app, bool deserialized) override;
        void on_event(App* app, Event& event) override;
    protected:
        bool on_window_resize(WindowResizeEvent& event) override;
    private:
        FreeCamera m_Camera;
    };
    
}