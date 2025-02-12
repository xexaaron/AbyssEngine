#include "Rendering/UI/Canvas.h"

namespace aby::ui {

    Ref<Canvas> Canvas::create(const glm::u32vec2& size, const Style& style) {
        return create_ref<Canvas>(size, style);
    }

    Canvas::Canvas(const glm::u32vec2& size, const Style& style) :
        m_BorderWidth(style.border.width),
        m_ContentArea(
            glm::vec2{
                size.x - (2 * m_BorderWidth),  // Subtract border on both sides
                size.y - (2 * m_BorderWidth)
            },
            glm::vec3{
                size.x / 2.f,  // Center the content area properly
                size.y / 2.f,
                0.f
            },
            style.bg.color,
            style.bg.texture.handle()
        ),
        m_Border(
            glm::vec2{ size.x, size.y },  // Border should match full size
            glm::vec3{ size.x / 2.f, size.y / 2.f, 0.f },
            style.border.color,
            0.f
        ) {
    }

    void Canvas::on_tick(App* app, Time deltatime) {
        auto& ren = app->renderer();
        ren.on_begin();
        ren.draw_quad_2d(m_Border);
        ren.draw_quad_2d(m_ContentArea);
        WidgetParent::on_tick(app, deltatime);
        ren.on_end();
    }
    void Canvas::on_event(App* app, Event& event) {
        EventDispatcher dsp(event);
        dsp.bind(this, &Canvas::on_window_resize);
        WidgetParent::on_event(app, event);
    }
    void Canvas::on_resize(const glm::u32vec2& new_size) {
        glm::u32vec2 old_size = m_ContentArea.size;

        // Ensure content area stays within the border
        m_ContentArea.size = {
            new_size.x > 2 * m_BorderWidth ? new_size.x - (2 * m_BorderWidth) : 0,
            new_size.y > 2 * m_BorderWidth ? new_size.y - (2 * m_BorderWidth) : 0
        };

        m_ContentArea.v.pos = { new_size.x / 2.f, new_size.y / 2.f, 0.f };
        m_Border.size = new_size;
        m_Border.v.pos = { new_size.x / 2.f, new_size.y / 2.f, 0.f };

        // Prevent division by zero
        if (old_size.x == 0 || old_size.y == 0 || m_ContentArea.size.x == 0 || m_ContentArea.size.y == 0) {
            Widget::on_resize(new_size);
            return;
        }

        glm::vec2 scale = glm::vec2(m_ContentArea.size) / glm::vec2(old_size);

        for (auto& widget : this->children()) {
            glm::u32vec2 child_size = glm::u32vec2(glm::vec2(widget->m_Size) * scale);
            widget->on_resize(child_size);
        }

        Widget::on_resize(new_size);
    }

    void Canvas::set_bg(const Background& bg) {
        m_ContentArea.v.col = bg.color;
        m_ContentArea.v.texinfo.z = bg.texture.handle();
    }
    
    void Canvas::set_border_width(float border_width) {
        m_BorderWidth = border_width;
        on_resize(m_Border.size);
    }
 
    bool Canvas::on_window_resize(WindowResizeEvent& event) {
        on_resize(event.size());
        return false;
    }

}