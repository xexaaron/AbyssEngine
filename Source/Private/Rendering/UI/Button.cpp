#include "Rendering/UI/Button.h"
#include "Core/Log.h"

namespace aby::ui {
    
    Ref<Button> Button::create(const Transform& transform, const ButtonStyle& style, const TextInfo& text_info, bool text_copyable) {
        return create_ref<Button>(transform, style, text_info, text_copyable);
    }

    Button::Button(const Transform& transform, const ButtonStyle& style, const TextInfo& text_info, bool text_copyable) :
        Textbox(transform, Style{ style.released, style.border }, text_info),
        m_Default(style.released),
        m_Hovered(style.hovered),
        m_Pressed(style.pressed),
        m_State(EButtonState::DEFAULT),
        m_Window(nullptr),
        bTextCopyable(text_copyable)
    {

    }

    void Button::on_create(App* app, bool deserialized) {
        Textbox::on_create(app, deserialized);
        m_Window = app->window();
    }

    void Button::on_tick(App* app, Time deltatime) {
        if (!bVisible) return;
        Textbox::on_tick(app, deltatime);
    }

    void Button::on_event(App* app, Event& event) {
        Widget::on_event(app, event);
        EventDispatcher dsp(event);
        dsp.bind<MouseMovedEvent>([this](MouseMovedEvent& event){ 
            return on_mouse_moved(event);
        });
        dsp.bind<MousePressedEvent>([this](MousePressedEvent& event){
            return on_mouse_pressed(event);
        });
        dsp.bind<MouseReleasedEvent>([this](MouseReleasedEvent& event){ 
            return on_mouse_released(event);
        });
    }

    void Button::on_pressed() {
    
    }
    
    void Button::on_released() {
    }
        
    void Button::on_hovered() {
    }

    void Button::on_unhovered() {
    }


    bool Button::on_mouse_moved(MouseMovedEvent& event) {
        bool hit = event.hit(m_Transform.position, m_Transform.size);
        if (!hit) {
            if (m_State == EButtonState::HOVERED) {
                m_Window->set_cursor(ECursor::ARROW);
                on_unhovered();
            }
            if (m_State != EButtonState::DEFAULT) {
                m_State = EButtonState::DEFAULT;
                invalidate_self();
            }
           
        }
        else {
            if (bTextCopyable && event.hit(m_Text.pos, m_TextSize)) {
                m_Window->set_cursor(ECursor::IBEAM);
            }
            else {
                if (m_State == EButtonState::DEFAULT) {
                    m_State = EButtonState::HOVERED;
                }
                m_Window->set_cursor(ECursor::HAND);
            }
            on_hovered();
            invalidate_self();
        }
        return false;
    }

    bool Button::on_mouse_pressed(MousePressedEvent& event) {
        bool hit = event.hit(m_Transform.position, m_Transform.size);
        if (hit) {
            m_State = EButtonState::PRESSED;
            on_pressed();
            invalidate_self();
        }
        return hit; // Stop propogating if we intercepted the hit.
    }
   
    bool Button::on_mouse_released(MouseReleasedEvent& event) {
        bool hit = event.hit(m_Transform.position, m_Transform.size);

        // Onl  y change state if the button was previously PRESSED
        if (m_State == EButtonState::PRESSED) {
            if (hit) {
                m_State = EButtonState::HOVERED;
                on_released(); // Trigger only if released inside
            }
            else {
                m_State = EButtonState::DEFAULT;
            }
            invalidate_self();
        }

        return hit; // Stop propagation if released inside
    }

    bool Button::on_invalidate() {
        if (bInvalid) {
            switch (m_State) {
                case EButtonState::DEFAULT:
                    m_Style.background = m_Default;
                    break;
                case EButtonState::HOVERED:
                    m_Style.background = m_Hovered;
                    break;
                case EButtonState::PRESSED:
                    m_Style.background = m_Pressed;
                    break;
            }

            return Textbox::on_invalidate();
        }
        return false;
    }

}