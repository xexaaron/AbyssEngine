#pragma once
#include "Rendering/UI/Textbox.h"

namespace aby::ui {

    class Button : public Textbox {
    public:
        static Ref<Button> create(const Transform& transform, const ButtonStyle& style, const TextInfo& text_info, bool text_copyable = false);
        
        Button(const Transform& transform, const ButtonStyle& style, const TextInfo& text_info, bool text_copyable = false);

        void on_create(App* app, bool deserialized) override;
        void on_tick(App* app, Time deltatime) override;
        void on_event(App* app, Event& event) override;

        virtual void on_pressed();
        virtual void on_released();
        virtual void on_hovered();
        virtual void on_unhovered();
    
        ButtonStyle button_style() const;
    protected:
        virtual bool on_mouse_moved(MouseMovedEvent& event);
        virtual bool on_mouse_pressed(MousePressedEvent& event);
        virtual bool on_mouse_released(MouseReleasedEvent& event);
    private:
        Background   m_Default;
        Background   m_Hovered;
        Background   m_Pressed;
    protected:
        Window*      m_Window;
        bool         bTextCopyable;
        EButtonState m_State;
    };

}