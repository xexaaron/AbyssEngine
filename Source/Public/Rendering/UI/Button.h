#pragma once
#include "Rendering/UI/Textbox.h"

namespace aby::ui {

    class Button : public Textbox {
    public:
        static Ref<Button> create(const Transform& transform, const ButtonStyle& style, const std::string& text);
        
        Button(const Transform& transform, const ButtonStyle& style, const std::string& text);

        void on_tick(App* app, Time deltatime) override;
        bool on_invalidate() override;
        void on_event(App* app, Event& event) override;

        virtual void on_pressed();
        virtual void on_released();
    private:
        bool on_mouse_moved(MouseMovedEvent& event);
        bool on_mouse_pressed(MousePressedEvent& event);
        bool on_mouse_released(MouseReleasedEvent& event);
    private:
        Background   m_Default;
        Background   m_Hovered;
        Background   m_Pressed;
        EButtonState m_State;
    };

}