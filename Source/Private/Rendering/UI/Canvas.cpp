#include "Rendering/UI/Canvas.h"

namespace aby::ui {
    
    Ref<Canvas> Canvas::create(const Style& style) {
        return create_ref<Canvas>(style);
    }

    Canvas::Canvas(const Style& style) :
        Panel(Transform{
            .position = { 0, 0 },
            .size = { 800, 600 }
        }, style, EResize::NONE)
    {

    }

    void Canvas::on_tick(App* app, Time deltatime) {
        if (!bVisible) return;
        on_invalidate();
        app->renderer().on_begin(m_Camera.view_projection());
        Panel::on_tick(app, deltatime);
        m_Camera.on_tick(app, deltatime);
        app->renderer().on_end();
    }

    void Canvas::on_event(App* app, Event& event) {
        Panel::on_event(app, event);
        m_Camera.on_event(app, event);
    }

    void Canvas::on_create(App* app, bool deserialized) {
        m_Transform.size = app->window()->size();
        Panel::on_create(app, deserialized);
        m_Camera.on_create(app, false);
    }

    bool Canvas::on_window_resize(WindowResizeEvent& event) {
        this->set_size(event.size());
        invalidate_self();
        return false;
    }

}