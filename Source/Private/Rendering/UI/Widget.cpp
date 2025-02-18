#include "Rendering/UI/Widget.h"
#include "Core/Log.h"
// widget
namespace aby::ui {

    Widget::Widget(const Transform& transform, const Style& style, Ref<Widget> parent) : 
        m_Transform(transform),
        m_Style(style),
        bInvalid(true),
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

    bool Widget::on_window_resize(WindowResizeEvent& event) {
        glm::vec2 old_size = event.old_size();
        glm::vec2 new_size = event.size();

        if (old_size.x == 0 || old_size.y == 0) {
            return false;
        }

        auto scale = new_size / old_size;
        // m_Transform.size *= scale;
        m_Transform.position *= scale;
        bInvalid = true;
        return false;
    }


}

//widget prrent
namespace aby::ui {
    ParentWidget::ParentWidget(std::size_t reserve) :
        m_Children(reserve) {}

    void ParentWidget::on_create(App* app, bool deserialized) {
        for (auto& widget : m_Children) {
            widget->on_create(app, deserialized);
        }
    }

    void ParentWidget::on_invalidate() {
        bool needs_re_sort = false;
        std::int32_t last_zindex = std::numeric_limits<std::int32_t>::min(); // Ensure valid initial comparison

        std::size_t invalid_size = m_Invalidated.size();
        while (invalid_size--) {  // Process all invalidated children
            auto index = m_Invalidated.front();
            m_Invalidated.pop();

            if (index >= m_Children.size())
                continue;

            if (auto& child = m_Children[index]; child) {
                if (child->is_invalid()) {
                    child->set_invalid(child->on_invalidate());
                }
                if (child->zindex() < last_zindex) {
                    needs_re_sort = true;
                }
                last_zindex = child->zindex(); // Update last seen zindex
            }
        }

        if (needs_re_sort) {
            std::stable_sort(m_Children.begin(), m_Children.end(), [](const Ref<Widget>& a, const Ref<Widget>& b) {
                return a->zindex() < b->zindex();
            });
        }
    }

    void ParentWidget::on_tick(App* app, Time deltatime) {
        for (std::size_t i = 0; i < m_Children.size(); i++) {
            auto& child = m_Children[i];
            child->on_tick(app, deltatime);
            if (child->is_invalid()) {
                m_Invalidated.push(i);
            }
        }
    }

    void ParentWidget::on_event(App* app, Event& event) {
        for (auto& child : m_Children) {
            child->on_event(app, event);
        }
    }

    void ParentWidget::on_destroy(App* app) {
        for (auto& child : m_Children) {
            child->on_destroy(app);
        }
    }

    std::size_t ParentWidget::add_child(Ref<Widget> child) {
        auto idx = m_Children.size();
        child->set_parent(shared_from_this());
        m_Children.push_back(child);
        return idx;
    }

    void ParentWidget::remove_child(Ref<Widget> child) {
        auto it = std::find(m_Children.begin(), m_Children.end(), child);
        ABY_ASSERT(it != m_Children.end(), "Child '{}' does not exist in parent '{}'", child->uuid(), this->uuid());
        std::size_t idx = std::distance(m_Children.begin(), it);
        m_Children.erase(it);
        std::queue<std::size_t> new_invalidated;
        while (!m_Invalidated.empty()) {
            auto invalidated_idx = m_Invalidated.front();
            m_Invalidated.pop();

            if (invalidated_idx > idx) {
                new_invalidated.push(invalidated_idx - 1);
            }
        }
        m_Invalidated = std::move(new_invalidated);
    }

    void ParentWidget::remove_child(std::size_t idx) {
        ABY_ASSERT(idx < m_Children.size(), "Out of bounds");
        m_Children.erase(m_Children.begin() + idx);
        std::queue<std::size_t> new_invalidated;
        while (!m_Invalidated.empty()) {
            auto invalidated_idx = m_Invalidated.front();
            m_Invalidated.pop();

            if (invalidated_idx > idx) {
                new_invalidated.push(invalidated_idx - 1);
            }
        }
        m_Invalidated = std::move(new_invalidated);
    }

    std::span<Ref<Widget>> ParentWidget::children() {
        return std::span(m_Children.begin(), m_Children.size());
    }

    std::span<const Ref<Widget>> ParentWidget::children() const {
        return std::span(m_Children.cbegin(), m_Children.size());
    }

}


// image

namespace aby::ui {
    Ref<Image> Image::create(const Transform& transform, const Style& style, Ref<Widget> parent) {
        return create_ref<Image>(transform, style, parent);
    }

    Image::Image(const Transform& transform, const Style& style, Ref<Widget> parent) :
        Widget(transform, style, parent),
        m_BorderCt(0) 
    {
    }

    void Image::on_tick(App* app, Time deltatime) {
        for (std::size_t i = 0; i < m_BorderCt; i++) {
            app->renderer().draw_quad_2d(m_Border[i]);
        }
        app->renderer().draw_quad_2d(m_Image);
    }

    bool Image::on_invalidate() {
        if (m_Transform.anchor.position != EAnchor::NONE) {
            m_Transform.position = m_Transform.anchor.offset;
            auto parent = m_Parent.lock();
            ABY_ASSERT(parent, "aby::ui::Image has no parent to be anchor to");
            auto parent_widget = parent->as<Widget>();
            auto parent_size = parent_widget->transform().size;
            switch (m_Transform.anchor.position) {
                case EAnchor::TOP_MIDDLE:
                    m_Transform.position.x += (parent_size.x / 2) - (m_Transform.size.x / 2);
                    break;
                case EAnchor::TOP_RIGHT:
                    m_Transform.position.x += (parent_size.x) - (m_Transform.size.x);
                    break;
                case EAnchor::MIDDLE_LEFT:
                    m_Transform.position.y += (parent_size.y / 2) - (m_Transform.size.y / 2);
                    break;
                case EAnchor::MIDDLE:
                    m_Transform.position.x += (parent_size.x / 2) - (m_Transform.size.x / 2);
                    m_Transform.position.y += (parent_size.y / 2) - (m_Transform.size.y / 2);
                    break;
                case EAnchor::MIDDLE_RIGHT:
                    m_Transform.position.x += (parent_size.x) - (m_Transform.size.x);
                    m_Transform.position.y += (parent_size.y / 2) - (m_Transform.size.y / 2);
                    break;
                case EAnchor::BOTTOM_LEFT:
                    m_Transform.position.y += (parent_size.y) - (m_Transform.size.y);
                    break;
                case EAnchor::BOTTOM_MIDDLE:
                    m_Transform.position.x += (parent_size.x / 2) - (m_Transform.size.x /2);
                    m_Transform.position.y += (parent_size.y) - (m_Transform.size.y);
                    break;
                case EAnchor::BOTTOM_RIGHT:
                    m_Transform.position.x += (parent_size.x) - (m_Transform.size.x);
                    m_Transform.position.y += (parent_size.y) - (m_Transform.size.y);
                    break;
            }
        }

        m_Image = Quad(
            m_Transform.size - glm::vec2(m_Style.border.width * 2.f),
            m_Transform.position + glm::vec2(m_Style.border.width),
            m_Style.background.color,
            static_cast<float>(m_Style.background.texture.handle())
        );
        if (m_Style.background.color.a <= 0.f) {
            m_Border[0] = Quad(
                glm::vec2(m_Transform.size.x, m_Style.border.width),
                m_Transform.position,
                m_Style.border.color,
                0.f
            );
            // Bottom border
            m_Border[1] = Quad(
                glm::vec2(m_Transform.size.x, m_Style.border.width),
                m_Transform.position + glm::vec2(0.f, m_Transform.size.y - m_Style.border.width),
                m_Style.border.color,
                0.f
            );
            // Left border
            m_Border[2] = Quad(
                glm::vec2(m_Style.border.width, m_Transform.size.y),
                m_Transform.position,
                m_Style.border.color,
                0.f
            );
            // Right border
            m_Border[3] = Quad(
                glm::vec2(m_Style.border.width, m_Transform.size.y),
                m_Transform.position + glm::vec2(m_Transform.size.x - m_Style.border.width, 0.f),
                m_Style.border.color,
                0.f
            );
            m_BorderCt = 4;
        }
        else {
            m_Border[0] = Quad(
                m_Transform.size,
                m_Transform.position,
                m_Style.border.color,
                0.f
            );
            m_BorderCt = 1;

        }

        return false;
    }
}

// Panel 

namespace aby::ui {

    Ref<Panel> Panel::create(const Transform& transform, const Style& style) {
        return create_ref<Panel>(transform, style);
    }

    Panel::Panel(const Transform& transform, const Style& style) :
        ParentWidget(0),
        Image(transform, style) {}

    void Panel::on_create(App* app, bool deserialized) {
        m_Transform.size = app->window()->size();
        Image::on_create(app, deserialized);
        bInvalid = Image::on_invalidate();
        ParentWidget::on_create(app, deserialized);
    }

    void Panel::on_tick(App* app, Time deltatime) {
        Image::on_tick(app, deltatime);
        ParentWidget::on_tick(app, deltatime);
    }

    void Panel::on_event(App* app, Event& event) {
        Image::on_event(app, event);
        ParentWidget::on_event(app, event);
    }

    void Panel::on_destroy(App* app) {
        Image::on_destroy(app);
        ParentWidget::on_destroy(app);
    }

    bool Panel::on_invalidate() {
        if (bInvalid) {
            bInvalid = Image::on_invalidate();
        }
        ParentWidget::on_invalidate();
        return bInvalid;
    }

}


// canvas
namespace aby::ui {
    
    Ref<Canvas> Canvas::create(const Style& style) {
        return create_ref<Canvas>(style);
    }

    Canvas::Canvas(const Style& style) :
        Panel(Transform{
            .position = { 0, 0 },
            .size = { 800, 600 }
        }, style)
    {

    }

    void Canvas::on_tick(App* app, Time deltatime) {
        on_invalidate();
        app->renderer().on_begin();
        Panel::on_tick(app, deltatime);
        app->renderer().on_end();
    }

    void Canvas::on_create(App* app, bool deserialized) {
        m_Transform.size = app->window()->size();
        Panel::on_create(app, deserialized);
    }

    bool Canvas::on_window_resize(WindowResizeEvent& event) {
        this->set_size(event.size());
        on_invalidate();
        return false;
    }

}


namespace aby::ui {
    Ref<Textbox> Textbox::create(const Transform& transform, const Style& style, const std::string& text, float scale) {
        return create_ref<Textbox>(transform, style, text, scale);
    }

    Textbox::Textbox(const Transform& transform, const Style& style, const std::string& text, float scale) :
        Image(transform, style),
        m_Text(text, transform.position + (transform.size / glm::vec2(2, 2)), scale, 0),
        m_TextSize(0, 0)
    {
    }

    void Textbox::on_create(App* app, bool deserialized) {
        Image::on_create(app, deserialized);
        m_Font = app->ctx().fonts().at(Resource{ EResource::FONT, m_Text.font });
        on_invalidate();
    }

    void Textbox::on_tick(App* app, Time deltatime) {
        Image::on_tick(app, deltatime);
        app->renderer().draw_text(m_Text);
    }
            
    bool Textbox::on_invalidate() {
        auto result = Image::on_invalidate();

        m_TextSize = m_Font->measure(m_Text.text) * m_Text.scale;
        // if (m_TextSize.x > m_Transform.size.x || m_TextSize.y > m_Transform.size.y) {
        //     float scaleX = m_Transform.size.x / m_TextSize.x;
        //     float scaleY = m_Transform.size.y / m_TextSize.y;
        //     m_Text.scale = glm::min(scaleX, scaleY);  // Shrink to fit
        //     m_TextSize = m_Font->measure(m_Text.text) * m_Text.scale;
        // }
        glm::vec3 offset = glm::vec3(m_TextSize / 2.f, 0.f);
        m_Text.pos = glm::vec3(m_Transform.position + (m_Transform.size / 2.f), 0.f) - offset;

        return result;
    }
    void Textbox::set_text(const std::string& text, Ref<Font> font) {
        m_Text.text = text;
        m_TextSize  = font->measure(text);
    }

}

namespace aby::ui {
    
    Ref<Button> Button::create(const Transform& transform, const ButtonStyle& style, const std::string& text) {
        return create_ref<Button>(transform, style, text);
    }

    Button::Button(const Transform& transform, const ButtonStyle& style, const std::string& text) :
        Textbox(transform, { style.released, style.border }, text, 1.f),
        m_Default(style.released),
        m_Hovered(style.hovered),
        m_Pressed(style.pressed),
        m_State(EButtonState::DEFAULT)
    {

    }

    void Button::on_tick(App* app, Time deltatime) {
        Textbox::on_tick(app, deltatime);
    }

    void Button::on_event(App* app, Event& event) {
        Widget::on_event(app, event);
        EventDispatcher dsp(event);
        dsp.bind(this, &Button::on_mouse_moved);
        dsp.bind(this, &Button::on_mouse_pressed);
        dsp.bind(this, &Button::on_mouse_released);

    }

    void Button::on_pressed() {
        ABY_LOG("Button '{}' pressed", this->uuid());
    }
    
    void Button::on_released() {
        ABY_LOG("Button '{}' released", this->uuid());
    }
        
    bool Button::on_mouse_moved(MouseMovedEvent& event) {
        bool hit = event.hit(m_Transform.position, m_Transform.size);
        if (hit) {
            if (m_State == EButtonState::DEFAULT) {
                m_State = EButtonState::HOVERED;
            }
            bInvalid = true;
        }
        else {
            if (m_State != EButtonState::DEFAULT) {
                m_State = EButtonState::DEFAULT;
                bInvalid = true;
            }
        }
        return false;
    }

    bool Button::on_mouse_pressed(MousePressedEvent& event) {
        bool hit = event.hit(m_Transform.position, m_Transform.size);
        if (hit) {
            m_State = EButtonState::PRESSED;
            on_pressed();
            bInvalid = true;
        }
        return hit; // Stop propogating if we intercepted the hit.
    }
   
    bool Button::on_mouse_released(MouseReleasedEvent& event) {
        bool hit = event.hit(m_Transform.position, m_Transform.size);

        // Onl  y change state if the button was previously PRESSED
        if (m_State == EButtonState::PRESSED) {
            if (hit) {
                m_State = EButtonState::HOVERED;
                on_released(); // Trigger only if released inside
            }
            else {
                m_State = EButtonState::DEFAULT;
            }
            bInvalid = true;
        }

        return hit; // Stop propagation if released inside
    }

    bool Button::on_invalidate() {
        if (bInvalid) {
            switch (m_State) {
                case EButtonState::DEFAULT:
                    m_Style.background = m_Default;
                    break;
                case EButtonState::HOVERED:
                    m_Style.background = m_Hovered;
                    break;
                case EButtonState::PRESSED:
                    m_Style.background = m_Pressed;
                    break;
            }

            bool result = Textbox::on_invalidate();
            return result;
        }
        return false;
    }

}


// LayoutContainer
namespace aby::ui {
    
    Ref<LayoutContainer> LayoutContainer::create(const Transform& transform, const Style& style, EDirection direction) {
        return create_ref<LayoutContainer>(transform, style, direction);
    }

    LayoutContainer::LayoutContainer(const Transform& transform, const Style& style, EDirection direction) :
        Panel(transform, style),
        m_Direction(direction)
    {
        
    }

    std::size_t LayoutContainer::add_child(Ref<Widget> widget) {
        auto idx = Panel::add_child(widget);
        align(widget);
        return idx;
    }

    // void LayoutContainer::remove_child(Ref<Widget> widget) {
    // 
    // }
    // 
    // void LayoutContainer::remove_child(Ref<Widget> widget) {
    // 
    // }

    void LayoutContainer::align(Ref<Widget> widget) {
        auto vector_idx = static_cast<std::size_t>(m_Direction);
        auto widget_pos = m_Transform.position;
        for (auto& child : children()) {
            widget_pos[vector_idx] += child->transform().size[vector_idx];
        }
         
        widget->set_position(widget_pos);
        if (widget_pos[vector_idx] + widget->transform().size[vector_idx] != m_Transform.size[vector_idx]) {
            scale();
        }
    }

    void LayoutContainer::scale() {
        auto vector_idx = static_cast<std::size_t>(m_Direction);
        float total_size = 0.f;

        for (auto& child : children()) {
            total_size += child->transform().size[vector_idx];
        }

        float delta = m_Transform.size[vector_idx] - total_size;
        if (delta == 0.f || children().empty()) return;

        float per_child_delta = delta / children().size();

        for (auto& child : children()) {
            auto new_size = child->transform().size;
            new_size[vector_idx] = std::max(new_size[vector_idx] + per_child_delta, 0.f);  // Prevent negative sizes
            child->set_size(new_size);
        }
    }

    bool LayoutContainer::on_invalidate() {
        scale();
        return Panel::on_invalidate();
    }
    

}