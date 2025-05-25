#include "Widget/Canvas.h"

namespace aby::ui {
    
    Ref<Canvas> Canvas::create(const ImageStyle& style) {
        return create_ref<Canvas>(style);
    }

    Canvas::Canvas(const ImageStyle& style) :
        Panel(Transform{
                .position = { 0, 0 },
                .size = { 800, 600 }
            }, ImageStyle{ .border={ .color = {0.f, 0.f, 0.f, 0.f },.width = 0.f}, .color = {0.f,0.f,0.f,0.f}, .texture = style.texture }, EResize::NONE)
    {
        m_Name = "Canvas";
    }

    void Canvas::on_create(App* app, bool deserialized) {
        m_Transform.size = app->window()->size();
        Panel::on_create(app, deserialized);
        m_Camera.on_create(app, false);
        m_Camera.set_viewport(app->window()->size());
    }

    void Canvas::on_tick(App* app, Time deltatime) {
        if (!bVisible) return;
        //layout_widgets();
        auto& ren = app->renderer();
        m_Camera.on_tick(app, deltatime);
        ren.on_begin(m_Camera.view_projection());
        Panel::on_tick(app, deltatime);
        Quad quad({ 100, 100, 100 }, {0, 0, 0});
        ren.draw_cube(quad);
        ren.on_end();
    }

    void Canvas::on_event(App* app, Event& event) {
        Panel::on_event(app, event);
        m_Camera.on_event(app, event);
    }

    void Canvas::layout_widgets() {
        for_each([&](Ref<Widget> widget) {
            layout_widgets(m_Transform.size, m_Transform.position, widget);
        });
    }

    void Canvas::layout_widgets(const glm::vec2& parent_size, const glm::vec2& parent_pos, Ref<Widget> widget) {
        if (widget->m_Transform.anchor.position != EAnchor::NONE) {
            anchor_widget(parent_size, parent_pos, widget);
        }
        widget->for_each([&](Ref<Widget> child) {
            layout_widgets(widget->m_Transform.size, widget->m_Transform.position, child);
        });
    }

    void Canvas::anchor_widget(const glm::vec2& parent_size, const glm::vec2& parent_pos, Ref<Widget> widget) {
        switch (widget->m_Transform.anchor.position) {
            case EAnchor::TOP_LEFT:
                widget->m_Transform.position = (parent_pos);
                break;
            case EAnchor::TOP_CENTER:
                widget->m_Transform.position = (glm::vec2{
                    parent_pos.x + (parent_size.x / 2) - (widget->m_Transform.size.x / 2),
                    parent_pos.y
                    });
                break;
            case EAnchor::TOP_RIGHT:
                widget->m_Transform.position = (glm::vec2{
                    parent_pos.x + parent_size.x - widget->m_Transform.size.x,
                    parent_pos.y
                    });
                break;
            case EAnchor::CENTER_LEFT:
                widget->m_Transform.position = (glm::vec2{
                    parent_pos.x,
                    parent_pos.y + (parent_size.y / 2) - (widget->m_Transform.size.y / 2)
                });
                break;
            case EAnchor::CENTER:
                widget->m_Transform.position = (glm::vec2{
                    parent_pos.x + (parent_size.x / 2) - (widget->m_Transform.size.x / 2),
                    parent_pos.y + (parent_size.y / 2) - (widget->m_Transform.size.y / 2)
                    });
                break;
            case EAnchor::CENTER_RIGHT:
                widget->m_Transform.position = (glm::vec2{
                    parent_pos.x + parent_size.x - widget->m_Transform.size.x,
                    parent_pos.y + (parent_size.y / 2) - (widget->m_Transform.size.y / 2)
                });
                break;
            case EAnchor::BOTTOM_LEFT:
                widget->m_Transform.position = (glm::vec2{
                    parent_pos.x,
                    parent_pos.y + parent_size.y - widget->m_Transform.size.y
                });
                break;
            case EAnchor::BOTTOM_CENTER:
                widget->m_Transform.position = (glm::vec2{
                    parent_pos.x + (parent_size.x / 2) - (widget->m_Transform.size.x / 2),
                    parent_pos.y + parent_size.y - widget->m_Transform.size.y
                    });
                break;
            case EAnchor::BOTTOM_RIGHT:
                widget->m_Transform.position = (glm::vec2{
                    parent_pos.x + parent_size.x - widget->m_Transform.size.x,
                    parent_pos.y + parent_size.y - widget->m_Transform.size.y
                });
                break;
            default:
                break;
        }
    }

    bool Canvas::on_window_resize(WindowResizeEvent& event) {
        this->set_size(event.size());
        m_Camera.set_viewport(event.size());
        layout_widgets();
        return false;
    }

}