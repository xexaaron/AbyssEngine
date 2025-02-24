#pragma once
#include "Rendering/UI/Panel.h"

namespace aby::ui {
    
    class LayoutContainer : public Panel {
    public:
        static Ref<LayoutContainer> create(const Transform& transform, const Style& style, EDirection direction, ELayout layout = ELayout::AUTO, float padding = 0.f);
        LayoutContainer(const Transform& transform, const Style& style, EDirection direction, ELayout layout = ELayout::AUTO, float padding = 0.f);
        virtual ~LayoutContainer() = default;

        void on_tick(App* app, Time deltatime) override;
        bool on_invalidate() override;
        
        void align();
        std::size_t add_child(Ref<Widget> widget) override;
    protected:
    private:
        EDirection m_Direction;
        ELayout    m_Layout;
        float      m_Padding;
    };
    
}