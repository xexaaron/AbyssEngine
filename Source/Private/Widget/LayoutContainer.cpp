#include "Widget/LayoutContainer.h"

namespace aby::ui {
    
    Ref<LayoutContainer> LayoutContainer::create(const Transform& transform, const ImageStyle& style, EDirection direction, ELayout layout, float padding) {
        return create_ref<LayoutContainer>(transform, style, direction, layout, padding);
    }

    LayoutContainer::LayoutContainer(const Transform& transform, const ImageStyle& style, EDirection direction, ELayout layout , float padding) :
        Panel(transform, style, EResize::NONE),
        m_Direction(direction),
        m_Layout(layout),
        m_Padding(padding)
    {
        m_Transform.size.y += padding * 2;
        if (m_Layout == ELayout::AUTO) {
            if (m_Direction == EDirection::HORIZONTAL) {
                m_Layout = ELayout::LEFT_TO_RIGHT;
            }
            else {
                m_Layout = ELayout::TOP_TO_BOTTOM;
            }
        }
        m_Name = "LayoutContainer";
    }
    
    std::size_t LayoutContainer::add_child(Ref<Widget> widget) {
        auto idx = m_Children.size();
        Widget::add_child(widget);
        auto dir = static_cast<int>(m_Direction);

        auto pos = m_Transform.position + m_Padding;
        if (m_Layout == ELayout::RIGHT_TO_LEFT || m_Layout == ELayout::BOTTOM_TO_TOP) {
            pos = m_Transform.position + m_Transform.size - m_Padding;
        }

        for_each([&](Ref<Widget> widget) {
            if (widget.get() == widget.get()) return; // Stop before newly added widget
            if (m_Layout == ELayout::RIGHT_TO_LEFT || m_Layout == ELayout::BOTTOM_TO_TOP) {
                pos[dir] -= widget->transform().size[dir];
            }
            else {
                pos[dir] += widget->transform().size[dir];
            }
        });

        widget->set_position(pos);
        return idx;
    }


    void LayoutContainer::on_tick(App* app, Time deltatime) {
        if (!bVisible) return;      
        align();
        Panel::on_tick(app, deltatime);
    }

    void LayoutContainer::align() {
        if (m_Children.empty()) return;

        auto vector_idx = static_cast<bool>(m_Direction);

        auto widget_pos = m_Transform.position[vector_idx] + m_Padding;
        if (m_Layout == ELayout::RIGHT_TO_LEFT || m_Layout == ELayout::BOTTOM_TO_TOP) {
            widget_pos = m_Transform.position[vector_idx] + m_Transform.size[vector_idx] - m_Padding;
        }
        for_each([&](Ref<Widget> widget) {
            if (m_Layout == ELayout::RIGHT_TO_LEFT || m_Layout == ELayout::BOTTOM_TO_TOP) {
                widget_pos -= widget->transform().size[vector_idx];
                glm::vec2 new_position = widget->transform().position;
                new_position[vector_idx] = widget_pos;
                new_position[!vector_idx] = m_Transform.position[!vector_idx] + m_Padding;
                widget->set_position(new_position);
            }
            else {
                glm::vec2 new_position = widget->transform().position;
                new_position[vector_idx] = widget_pos;
                new_position[!vector_idx] = m_Transform.position[!vector_idx] + m_Padding;
                widget->set_position(new_position);
                widget_pos += widget->transform().size[vector_idx];
            }
        });
    }

}