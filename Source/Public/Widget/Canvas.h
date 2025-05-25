#pragma once
#include "Widget/Panel.h"
#include "Rendering/Camera.h"

namespace aby::ui {
    
    class Canvas : public Panel {
    public:
        static Ref<Canvas> create(const ImageStyle& style);
        Canvas(const ImageStyle& style);
        virtual ~Canvas() = default;
        
        void on_tick(App* app, Time deltatime) override;
        void on_create(App* app, bool deserialized) override;
        void on_event(App* app, Event& event) override;

    protected:
        void layout_widgets();
        void layout_widgets(const glm::vec2& parent_size, const glm::vec2& parent_pos, Ref<Widget> widget);
        void anchor_widget(const glm::vec2& parent_size, const glm::vec2& parent_pos, Ref<Widget> widget);

        bool on_window_resize(WindowResizeEvent& event) override;
    private:
        OrientedCamera m_Camera;
    };
    
}