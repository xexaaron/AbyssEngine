#include "Widget/InputTextbox.h"
#include "Utility/Inserter.h"

namespace aby::ui {


	Ref<InputTextbox> InputTextbox::create(const Transform& transform, const ImageStyle& style, const TextInfo& info, InputTextOptions opts) {
		return create_ref<InputTextbox>(transform, style, info, opts);
	}

	InputTextbox::InputTextbox(const Transform& transform, const ImageStyle& style, const TextInfo& info, InputTextOptions opts) :
		Button(transform, ButtonStyle{ 
				.hovered  = { style.color, style.texture },
				.pressed  = { style.color, style.texture },
				.released = { style.color, style.texture },
				.border   = style.border 
			}, info, true
		),
		bFocused(false),
		m_Opts(opts),
		m_Cursor(m_Text.text)
	{
		m_Name = "InputTextbox";
	}

	void InputTextbox::on_tick(App* app, Time deltatime) {
		if (!bVisible) return;
		Button::on_tick(app, deltatime);
		if (m_Opts.cursor) {
			if (bFocused) {
				Text text = m_Text;
				text.text = m_Cursor.format();
				app->renderer().draw_text(text);
			}
			else {
				app->renderer().draw_text(m_Text);
			}
		}
		else {
			app->renderer().draw_text(m_Text);
		}
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
		if (bFocused) {
			char character = static_cast<char>(event.code());
			if (m_Opts.cursor) {
				m_Cursor.insert_at(character);
			}
			else {
				m_Text.text.append(1, character);
			}
			if (auto font = m_Font.lock()) {
				if (font->is_mono()) {
					m_TextSize.x += font->char_width();
				}
				else {
					m_TextSize = font->measure(m_Text.prefix + m_Text.text);
				}
			}
			return true;
		}
		return false;
	}

	bool InputTextbox::on_key_pressed(KeyPressedEvent& event) {
		if (bFocused) {
			if (event.code() == aby::Button::KEY_BACKSPACE) {
				if (!m_Text.text.empty()) {
					if (m_Opts.cursor) {
						m_Cursor.delete_at();
					}
					else {
						m_Text.text.pop_back();
					}
					if (auto font = m_Font.lock()) {
						if (font->is_mono()) {
							m_TextSize.x -= font->char_width();
						}
						else {
							m_TextSize = font->measure(m_Text.prefix + m_Text.text);
						}
					}
				}
			}
			else if (event.code() == aby::Button::KEY_ENTER) {
				if (m_Opts.submit_clears_focus) {
					bFocused = false;
				}
				on_submit(m_Text.text);
				if (m_Opts.submit_clears_text) {
					m_Text.text.clear();
				}
			}
			else if (event.code() == aby::Button::KEY_ARROW_RIGHT) {
				bool hl		   = (event.mods() & aby::Button::EMod::SHIFT) != aby::Button::EMod::NONE;
				bool move_word = (event.mods() & aby::Button::EMod::CTRL) != aby::Button::EMod::NONE;
				if (move_word) {
					m_Cursor.move_next(hl);
				}
				else {
					m_Cursor.move_right(1, hl);
				}
			}
			else if (event.code() == aby::Button::KEY_ARROW_LEFT) {
				bool hl		   = (event.mods() & aby::Button::EMod::SHIFT) != aby::Button::EMod::NONE;
				bool move_word = (event.mods() & aby::Button::EMod::CTRL) != aby::Button::EMod::NONE;
				if (move_word) {
					m_Cursor.move_previous(hl);
				}
				else {
					m_Cursor.move_left(1, hl);
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

		return hit; // Stop propagation if released inside
	}

	void InputTextbox::set_options(const InputTextOptions& options) {
		m_Opts = options;
	}
	
	void InputTextbox::set_focused(bool focused) {
		bFocused = focused;
	}

	void InputTextbox::set_cursor_pos(std::size_t position) {
		m_Cursor.move_to(position);
	}

}