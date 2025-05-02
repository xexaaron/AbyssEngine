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

    void Canvas::on_create(App* app, bool deserialized) {
        m_Transform.size = app->window()->size();
        Panel::on_create(app, deserialized);
        m_Camera.on_create(app, false);
        m_Camera.set_viewport(app->window()->size());
    }

    void Canvas::on_tick(App* app, Time deltatime) {
        if (!bVisible) return;
        on_invalidate();
        auto& ren = app->renderer();
        m_Camera.on_tick(app, deltatime);
        ren.on_begin(m_Camera.view_projection());
        Panel::on_tick(app, deltatime); // Render Canvas Elements.
        glm::vec3 pos = m_Camera.position() + (m_Camera.forward() * 500.f);
        Quad quad({ 100, 100, 100 }, pos);
        ren.draw_quad_3d(quad);
        ren.on_end();
    }

    void Canvas::on_event(App* app, Event& event) {
        Panel::on_event(app, event);
        m_Camera.on_event(app, event);
    }

    bool Canvas::on_window_resize(WindowResizeEvent& event) {
        this->set_size(event.size());
        invalidate_self();
        m_Camera.set_viewport(event.size());
        return false;
    }

}