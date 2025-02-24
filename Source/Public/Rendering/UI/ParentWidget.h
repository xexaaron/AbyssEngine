#pragma once
#include "Rendering/UI/Widget.h"
#include <set>

namespace aby::ui {
    
    class ParentWidget : public virtual Object, public std::enable_shared_from_this<ParentWidget> {
    protected:
        using container      = typename std::vector<Ref<Widget>>;
        using for_each_fn    = std::function<void(Ref<Widget>)>;
        using for_each_fn_i  = std::function<void(Ref<Widget>, std::size_t)>;
    public:
        explicit ParentWidget(std::size_t reserve = 0);
        virtual ~ParentWidget() = default;

        void on_create(App* app, bool deserialized) override;
        void on_tick(App* app, Time deltatime) override;
        void on_event(App* app, Event& event) override;
        void on_destroy(App* app) override;
        void on_invalidate();

        virtual std::size_t add_child(Ref<Widget> child);
        virtual void remove_child(std::size_t idx);
        virtual void for_each(for_each_fn&& fn);
        void for_each(for_each_fn_i&& fn);
        std::span<Ref<Widget>> children();
        std::span<const Ref<Widget>> children() const;
    protected:
        container m_Children;
    private:
        std::set<std::size_t> m_Invalidated;
    };
}