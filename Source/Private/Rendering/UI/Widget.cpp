#include "Rendering/UI/Widget.h"
#include "Core/Log.h"
// widget
namespace aby::ui {

    Widget::Widget(const Transform& transform, const Style& style, Ref<Widget> parent) : 
        m_Transform(transform),
        m_Style(style),
        bInvalid(true),
        bVisible(true),
        m_ZIndex(0),
        m_Parent(parent) {}

    void Widget::on_event(App* app, Event& event) {
        EventDispatcher dsp(event);
        // Use non member bind to ensure polymorphic access of on_window_resize.
        dsp.bind<WindowResizeEvent>([this](WindowResizeEvent& event) -> bool {
            return this->on_window_resize(std::forward<WindowResizeEvent&>(event));
        });
    }

    void Widget::set_size(const glm::vec2& size) { 
        m_Transform.size = size;
        bInvalid = true;
    }

    void Widget::set_position(const glm::vec2& pos) { 
        m_Transform.position = pos;
        m_Transform.anchor.position = EAnchor::NONE;
        bInvalid = true;
    }

    void Widget::set_transform(const Transform& transform) {
        if (m_Transform.position != transform.position) {
            m_Transform.anchor.position = EAnchor::NONE;
        }
        m_Transform = transform;
        bInvalid = true;
    }

    void Widget::set_style(const Style& style) {
        m_Style = style;
        bInvalid = true;
    }

    void Widget::set_parent(Ref<Object> parent) {
        m_Parent = parent;
        bInvalid = true;
    }

    void Widget::set_anchor(Anchor anchor) {
        m_Transform.anchor = anchor;
        bInvalid = true;
    }

    void Widget::set_invalid(bool invalid) {
        bInvalid = invalid;
    }

    void Widget::set_zindex(std::int32_t zindex) {
        m_ZIndex = zindex;
        bInvalid = true;
    }

    void Widget::set_visible(bool visible) {
        bVisible = visible;
    }

    bool Widget::is_visible() const {
        return bVisible;
    }

    bool Widget::is_invalid() const {
        return bInvalid;
    }

    const Transform& Widget::transform() const {
        return m_Transform;
    }
    
    const Style& Widget::style() const {
        return m_Style;
    }

    std::int32_t Widget::zindex() const {
        return m_ZIndex;
    }

    Weak<Object> Widget::parent() const { 
        return m_Parent;
    }
    
    Weak<Object> Widget::parent() {
        return m_Parent;
    }

    void Widget::invalidate_self() {
        bInvalid = true;
        bInvalid = on_invalidate();

    }

    bool Widget::on_window_resize(WindowResizeEvent& event) {
        glm::vec2 old_size = event.old_size();
        glm::vec2 new_size = event.size();

        if (old_size.x == 0 || old_size.y == 0) {
            return false;
        }

        auto scale = new_size / old_size;
        m_Transform.size.x   *= scale.x;
        m_Transform.position *= scale;
        invalidate_self();
        return false;
    }


}

