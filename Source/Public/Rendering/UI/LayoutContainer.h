#pragma once
#include "Rendering/UI/Panel.h"

namespace aby::ui {
    
    class LayoutContainer : public Panel {
    public:
        static Ref<LayoutContainer> create(const Transform& transform, const Style& style, EDirection direction);
        LayoutContainer(const Transform& transform, const Style& style, EDirection direction);
        virtual ~LayoutContainer() = default;

        bool on_invalidate() override;

        std::size_t add_child(Ref<Widget> widget) override;
        // void remove_child(Ref<Widget> widget) override;
        // void remove_child(Ref<Widget> widget) override;
    protected:
        void align(Ref<Widget> widget);
        void scale();
    private:
        EDirection m_Direction;
    };
    
}