#pragma once
#include "Rendering/UI/Widget.h"

namespace aby::ui {
    
    class ParentWidget : public virtual Object, public std::enable_shared_from_this<ParentWidget> {
    public:
        explicit ParentWidget(std::size_t reserve = 0);
        virtual ~ParentWidget() = default;

        void on_create(App* app, bool deserialized) override;
        void on_tick(App* app, Time deltatime) override;
        void on_event(App* app, Event& event) override;
        void on_destroy(App* app) override;
        void on_invalidate();

        virtual std::size_t add_child(Ref<Widget> child);
        virtual void remove_child(Ref<Widget> child);
        virtual void remove_child(std::size_t idx);
        
        std::span<Ref<Widget>> children();
        std::span<const Ref<Widget>> children() const;
    private:
        std::vector<Ref<Widget>> m_Children;
        std::queue<std::size_t> m_Invalidated;
    };
}