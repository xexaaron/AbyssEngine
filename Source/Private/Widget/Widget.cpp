#include "Widget/Widget.h"

namespace aby::ui {

    Widget::Widget(const Transform& transform, const std::string& name, Ref<Widget> parent) : 
        bVisible(true),
        bScalesWithWindow(true),
        m_ZIndex(0),
        m_Transform(transform),
        m_Name(name),
        m_Parent(parent) {}

    void Widget::on_create(App* app, bool deserialized) {
        for_each([&app, deserialized = deserialized](Ref<Widget> widget) {
            widget->on_create(app, deserialized);
        });
    }

    void Widget::on_tick(App* app, Time deltatime) {
        for_each([&app, deltatime = deltatime](Ref<Widget> widget) {
            widget->on_tick(app, deltatime);
        });
    }

    void Widget::on_event(App* app, Event& event) {
        EventDispatcher dsp(event);
        // Use non-member bind to ensure polymorphic access of on_window_resize.
        dsp.bind<WindowResizeEvent>([this](WindowResizeEvent& event) -> bool {
            return this->on_window_resize(std::forward<WindowResizeEvent&>(event));
        });
        for_each([&app, &event](Ref<Widget> widget) {
            widget->on_event(app, event);
        });
    }

    void Widget::on_destroy(App* app) {
        for_each([&app](Ref<Widget> widget) {
            widget->on_destroy(app);
        });
    }

    bool Widget::on_window_resize(WindowResizeEvent& event) {
        if (bScalesWithWindow) {
            glm::vec2 old_size = event.old_size();
            glm::vec2 new_size = event.size();
            if (old_size.x == 0 || old_size.y == 0) {
                return false;
            }
            auto scale = new_size / old_size;
            m_Transform.size.x *= scale.x;
            m_Transform.position *= scale;
        }
        return false;
    }


    void Widget::set_size(const glm::vec2& size) { 
        m_Transform.size = size;
    }

    void Widget::set_position(const glm::vec2& pos) { 
        m_Transform.position = pos;
        m_Transform.anchor.position = EAnchor::NONE;
    }

    void Widget::set_transform(const Transform& transform) {
        if (m_Transform.position != transform.position) {
            m_Transform.anchor.position = EAnchor::NONE;
        }
        m_Transform = transform;
    }

    void Widget::set_parent(Ref<Widget> parent) {
        m_Parent = parent;
    }

    void Widget::set_anchor(Anchor anchor) {
        m_Transform.anchor = anchor;
    }

    void Widget::set_zindex(i32 zindex) {
        m_ZIndex = zindex;
    }

    void Widget::set_visible(bool visible) {
        bVisible = visible;
    }

    void Widget::set_name(const std::string& name) {
        m_Name = name;
    }

    bool Widget::is_visible() const {
        return bVisible;
    }

    const Transform& Widget::transform() const {
        return m_Transform;
    }

    i32 Widget::zindex() const {
        return m_ZIndex;
    }

    Weak<Object> Widget::parent() const { 
        return m_Parent;
    }

    void Widget::for_each(std::function<void(Ref<Widget>)> fn) {
        for (auto& widget : m_Children) {
            fn(widget);
        }
    }

    void Widget::for_each(std::function<void(Ref<Widget>, std::size_t)> fn) {
        std::size_t i = 0;
        for_each([&i, fn = std::move(fn)](Ref<Widget> widget) {
            fn(widget, i++);
        });
    }

    Widget::Children& Widget::children() {
        return m_Children;
    }

    const Widget::Children& Widget::children() const {
        return m_Children;
    }
    
    std::size_t Widget::add_child(Ref<Widget> widget) {
        m_Children.push_back(widget);
        widget->set_parent(shared_from_this());
        return m_Children.size() - 1;
    }
    
    void Widget::remove_child(std::size_t i) {
        ABY_ASSERT(m_Children.size() > i, "Out of bounds: 0...{} -> {}", m_Children.size(), i);
        m_Children[i]->set_parent(nullptr);
        m_Children.erase(m_Children.begin() + i);
    }

    void Widget::print(std::ostream& os) {
        os << std::boolalpha;
        os << m_Name << " | " << uuid() << '\n';
        os << " -- Visible    : " << bVisible << '\n';
        os << " -- ZIndex     : " << m_ZIndex << '\n';
        os << " -- Transform  : " <<             '\n';
        os << " ---- Anchor   : " << std::to_string(m_Transform.anchor.position) << '\n';
        os << " ------ Offset : " << m_Transform.anchor.offset << '\n';
        os << " ---- Position : " << m_Transform.position << '\n';
        os << " ---- Size     : " << m_Transform.size << '\n';
        os << " ---- Parent   : " << (m_Parent.lock() ? (m_Parent.lock()->m_Name + " | " + m_Parent.lock()->uuid().operator std::string()) : "N/A") << '\n';
        os << " ---- Children : " << m_Children.size() << '\n';
    }

    void Widget::recurse(std::function<void(Ref<Widget>)> fn) {
        fn(shared_from_this());
        for_each([&fn](Ref<Widget> widget) {
            widget->recurse(fn);
        });
    }

    const std::string& Widget::name() const {
        return m_Name;
    }


}