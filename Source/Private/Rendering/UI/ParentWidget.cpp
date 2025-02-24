#include "Rendering/UI/ParentWidget.h"
#include "Core/Log.h"

#define for_children(name) for (auto name = this->begin(); name != this->end(); ++name) 


namespace aby::ui {



    ParentWidget::ParentWidget(std::size_t reserve) :
        m_Children(reserve) {}

    void ParentWidget::on_create(App* app, bool deserialized) {
        for_each([&app, deserialized = deserialized](auto widget) {
            widget->on_create(app, deserialized);
            widget->on_invalidate();
        });
    }

    void ParentWidget::on_invalidate() {
        bool needs_re_sort = false;
        std::int32_t last_zindex = std::numeric_limits<std::int32_t>::min();

        for (auto index : m_Invalidated) {
            if (index >= m_Children.size()) continue;

            if (auto& child = m_Children[index]; child) {
                if (child->is_invalid()) {
                    child->set_invalid(child->on_invalidate());
                }
                if (child->zindex() < last_zindex) {
                    needs_re_sort = true;
                }
                last_zindex = child->zindex();
            }
        }

        m_Invalidated.clear();

        if (needs_re_sort) {
            std::stable_sort(m_Children.begin(), m_Children.end(), [](const Ref<Widget>& a, const Ref<Widget>& b) {
                return a->zindex() < b->zindex();
            });
        }
    }

    void ParentWidget::on_tick(App* app, Time deltatime) {
        for_each([this, &app, deltatime = deltatime](auto widget, std::size_t i) {
            widget->on_tick(app, deltatime);
            if (widget->is_invalid()) {
                m_Invalidated.insert(i);
            }
        });
    }

    void ParentWidget::on_event(App* app, Event& event) {
        for_each([&app, &event](auto widget) {
            widget->on_event(app, event);
        });
    }

    void ParentWidget::on_destroy(App* app) {
        for_each([&app](auto widget) {
            widget->on_destroy(app);
        });
    }

    std::size_t ParentWidget::add_child(Ref<Widget> child) {
        auto idx = m_Children.size();
        child->set_parent(shared_from_this());
        m_Children.push_back(child);
        on_invalidate();
        return idx;
    }


    void ParentWidget::remove_child(std::size_t idx) {
        ABY_ASSERT(idx < m_Children.size(), "Out of bounds");
        m_Children.erase(m_Children.begin() + idx);
        std::set<std::size_t> new_invalidated;
        for (auto invalidated_idx : m_Invalidated) {
            if (invalidated_idx > idx) {
                new_invalidated.insert(invalidated_idx - 1);
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

    void ParentWidget::for_each(for_each_fn&& fn) {
        for (auto& child : m_Children) {
            fn(child);
        }
    }
    
    void ParentWidget::for_each(for_each_fn_i&& fn) {
        std::size_t i = 0;
        for_each([&i, fn](auto widget) {
            fn(widget, i++);
        });
    }

}