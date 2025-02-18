#pragma once
#include "Rendering/UI/Widget.h"
#include <array>

namespace aby::ui {
    
    class Image : public Widget {
    public:
        static Ref<Image> create(const Transform& transform, const Style& style, Ref<Widget> parent = nullptr);
        Image(const Transform& transform, const Style& style, Ref<Widget> parent = nullptr);

        void on_tick(App* app, Time deltatime) override;
        bool on_invalidate() override;
    protected:
        Quad m_Image;
        std::array<Quad, 4> m_Border;
        std::size_t m_BorderCt;
    };
}