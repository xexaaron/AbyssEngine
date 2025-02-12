#pragma once

#include "Rendering/UI/Widget.h"
#include <glm/glm.hpp>
#include <span>

namespace aby::ui {

    class Canvas final : public WidgetParent {
    public:
        static Ref<Canvas> create(const glm::u32vec2& size, const Style& style);

        Canvas(const glm::u32vec2& size, const Style& style);
        ~Canvas() = default;

        void on_tick(App* app, Time deltatime) override;
        void on_event(App* app, Event& event) override;
        void on_resize(const glm::u32vec2& new_size) override;

        void set_bg(const Background& bg);
        void set_border_width(float border_width);
    private:
        bool on_window_resize(WindowResizeEvent& event);
    private:
        float m_BorderWidth;
        Quad m_ContentArea;
        Quad m_Border;
    };

}