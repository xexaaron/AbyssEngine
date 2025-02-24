#include "Rendering/UI/InputTextbox.h"

namespace aby::ui {


	Ref<InputTextbox> InputTextbox::create(const Transform& transform, const Style& style, const TextInfo& info, InputTextOptions opts) {
		return create_ref<InputTextbox>(transform, style, info, opts);
	}

	InputTextbox::InputTextbox(const Transform& transform, const Style& style, const TextInfo& info, InputTextOptions opts) :
		Button(transform, ButtonStyle{ 
				.hovered  = style.background,
				.pressed  = style.background,
				.released = style.background,
				.border   = style.border 
			}, info, true
		),
		bFocused(false),
		m_Opts(opts)
	{

	}

	void InputTextbox::on_event(App* app, Event& event) {
		if (bVisible) {
			Button::on_event(app, event);
			EventDispatcher dsp(event);
			dsp.bind(this, &InputTextbox::on_key_typed);
			dsp.bind(this, &InputTextbox::on_key_pressed);
		}
	}


	void InputTextbox::on_pressed() {
	
	}

	void InputTextbox::on_released() {
		bFocused = true;
	}

	void InputTextbox::on_submit(const std::string& msg) {

	}

	bool InputTextbox::on_key_typed(KeyTypedEvent& event) {
		if (bFocused){
			m_Text.text.append(1, static_cast<char>(event.code()));
			bInvalid = true;
			return true;
		}
		return false;
	}

	bool InputTextbox::on_key_pressed(KeyPressedEvent& event) {
		if (bFocused) {
			if (event.code() == aby::Button::KEY_BACKSPACE) {
				if (!m_Text.text.empty() && m_Text.text.length() > m_Opts.prefix) {
					m_Text.text.pop_back();
					bInvalid = true;
				}
			}
			else if (event.code() == aby::Button::KEY_ENTER) {
				if (m_Opts.submit_clears_focus) {
					bFocused = false;
				}
				bInvalid = true;
				if (m_Opts.prefix) {
					on_submit(m_Text.text.substr(m_Opts.prefix));

				}
				else {
					on_submit(m_Text.text);
				}
				if (m_Opts.submit_clears_text) {
					m_Text.text.erase(m_Opts.prefix);
				}
			}
			return true;
		}
		return false;
	}

	bool InputTextbox::on_mouse_released(MouseReleasedEvent& event) {
		bool hit = event.hit(m_Transform.position, m_Transform.size);
		if (hit) {
			if (m_State == EButtonState::PRESSED) {

				m_State = EButtonState::HOVERED;
				on_released(); // Trigger only if released inside
			}
		}
		else {
			if (m_State == EButtonState::PRESSED) {
				m_State = EButtonState::DEFAULT;
			}
			bFocused = false;
		}
		invalidate_self();

		return hit; // Stop propagation if released inside
	}


}