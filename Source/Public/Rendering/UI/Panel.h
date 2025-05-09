#pragma once
#include "Rendering/UI/Image.h"

namespace aby::ui {

    class Panel : public Image {
    public:
        static Ref<Panel> create(const Transform& transform, const ImageStyle& style, EResize resizability = EResize::NONE);
        Panel(const Transform& transform, const ImageStyle& style, EResize resizability = EResize::NONE);
        virtual ~Panel() = default;

        void on_create(App* app, bool deserialized) override;
        void on_event(App* app, Event& event) override;
        virtual void on_resize(EResize direction, float distance);

        void set_resizability(EResize resizability);
    protected:
        virtual bool on_mouse_moved(MouseMovedEvent& event);
        virtual bool on_mouse_pressed(MousePressedEvent& event);
        virtual bool on_mouse_released(MouseReleasedEvent& event);
    private:
        ResizeOperation m_Resize;
        App* m_App;
    };

}