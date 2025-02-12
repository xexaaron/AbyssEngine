#include "Rendering/UI/Widget.h"

namespace aby::ui {
    Widget::Widget(Ref<Widget> parent) : m_Parent(parent) {}

    void Widget::on_create(App* app, bool deserialized) {}
    void Widget::on_event(App* app, Event& event) {}
    void Widget::on_tick(App* app, Time deltatime) {}
    void Widget::on_destroy(App* app) {}
    void Widget::on_resize(const glm::u32vec2& new_size) { m_Size = new_size; }

    const glm::u32vec2& Widget::size() { return m_Size; }
    Weak<Widget> Widget::parent() const { return m_Parent; }
    Weak<Widget> Widget::parent() { return m_Parent; }
    void Widget::set_size(const glm::u32vec2& size) { on_resize(size); }
}

namespace aby::ui {
    WidgetParent::WidgetParent(std::size_t reserve) : m_Children(reserve) {}

    void WidgetParent::on_create(App* app, bool deserialized) {
        for (auto& widget : m_Children) {
            // TODO: Allow for serialization.
            widget->on_create(app, false);
        }
    }
    void WidgetParent::on_event(App* app, Event& event) {
        for (auto& widget : m_Children) {
            widget->on_event(app, event);
        }
    }
    void WidgetParent::on_tick(App* app, Time deltatime){
        for (auto& widget : m_Children) {
            widget->on_tick(app, deltatime);
        }
    }
    void WidgetParent::on_destroy(App* app) {
        for (auto& widget : m_Children) {
            widget->on_destroy(app);
        }
    }
    
    void WidgetParent::on_resize(const glm::u32vec2& new_size) {
        if (m_Size.x == 0 || m_Size.y == 0) {
            m_Size = new_size;
            return;
        }
    
        glm::vec2 scale = glm::vec2(new_size) / glm::vec2(m_Size);
    
        for (auto& widget : m_Children) {
            glm::u32vec2 child_size = glm::u32vec2(glm::vec2(widget->m_Size) * scale);
            widget->on_resize(child_size);
        }
     
        Widget::on_resize(new_size);
    }



    void WidgetParent::add_child(Ref<Widget> widget) {
        widget->m_Parent = shared_from_this();
        m_Children.push_back(widget);
    }
    void WidgetParent::erase_child(std::size_t index) {
        ABY_ASSERT(index < m_Children.size(), "Out of bounds");
        m_Children.erase(m_Children.begin() + index);
    }

    std::span<Ref<Widget>> WidgetParent::children() {
        return std::span(m_Children.begin(), m_Children.size());
    }
    std::span<const Ref<Widget>> WidgetParent::children() const {
        return std::span(m_Children.begin(), m_Children.size());
    }

    auto WidgetParent::begin() {
        return m_Children.begin();
    }
    auto WidgetParent::begin() const {
        return m_Children.begin();
    }
    auto WidgetParent::end() {
        return m_Children.end();
    }
    auto WidgetParent::end() const {
        return m_Children.end();
    }
}