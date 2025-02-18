#include "Rendering/UI/Button.h"
#include "Core/Log.h"

namespace aby::ui {
    
    Ref<Button> Button::create(const Transform& transform, const ButtonStyle& style, const std::string& text) {
        return create_ref<Button>(transform, style, text);
    }

    Button::Button(const Transform& transform, const ButtonStyle& style, const std::string& text) :
        Textbox(transform, { style.released, style.border }, text, 1.f),
        m_Default(style.released),
        m_Hovered(style.hovered),
        m_Pressed(style.pressed),
        m_State(EButtonState::DEFAULT)
    {

    }

    void Button::on_tick(App* app, Time deltatime) {
        Textbox::on_tick(app, deltatime);
    }

    void Button::on_event(App* app, Event& event) {
        Widget::on_event(app, event);
        EventDispatcher dsp(event);
        dsp.bind(this, &Button::on_mouse_moved);
        dsp.bind(this, &Button::on_mouse_pressed);
        dsp.bind(this, &Button::on_mouse_released);

    }

    void Button::on_pressed() {
        ABY_LOG("Button '{}' pressed", this->uuid());
    }
    
    void Button::on_released() {
        ABY_LOG("Button '{}' released", this->uuid());
    }
        
    bool Button::on_mouse_moved(MouseMovedEvent& event) {
        bool hit = event.hit(m_Transform.position, m_Transform.size);
        if (hit) {
            if (m_State == EButtonState::DEFAULT) {
                m_State = EButtonState::HOVERED;
            }
            bInvalid = true;
        }
        else {
            if (m_State != EButtonState::DEFAULT) {
                m_State = EButtonState::DEFAULT;
                bInvalid = true;
            }
        }
        return false;
    }

    bool Button::on_mouse_pressed(MousePressedEvent& event) {
        bool hit = event.hit(m_Transform.position, m_Transform.size);
        if (hit) {
            m_State = EButtonState::PRESSED;
            on_pressed();
            bInvalid = true;
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
            bInvalid = true;
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

            bool result = Textbox::on_invalidate();
            return result;
        }
        return false;
    }

}