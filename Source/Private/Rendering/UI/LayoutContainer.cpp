#include "Rendering/UI/LayoutContainer.h"

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

        bool is_first = true;
        for (auto& child : children()) {
            if (is_first) {
                child->set_position(m_Transform.position);
                is_first = false;
            }
            else {
                widget_pos[vector_idx] += child->transform().size[vector_idx];
                child->set_position(widget_pos);
            }
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
        auto res = Panel::on_invalidate();
        for (auto& widget : children()) {
            align(widget);
        }
        return res;
    }
    

}