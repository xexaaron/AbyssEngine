#include "Rendering/UI/ParentWidget.h"
#include "Core/Log.h"

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