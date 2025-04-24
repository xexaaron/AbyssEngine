#include "Rendering/UI/InputTextbox.h"
#include "Utility/Inserter.h"

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
		m_Opts(opts),
		m_Cursor(m_Text.text)
	{

	}

	void InputTextbox::on_tick(App* app, Time deltatime) {
		if (!bVisible) return;
		Image::on_tick(app, deltatime);
		if (m_Opts.cursor) {
			if (bFocused) {
				Text text = m_Text;
				text.text = m_Cursor.format();
				//std::cout << "Cursor: " << m_Cursor.position() << " " << text.text << std::endl;
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
			bInvalid = true;
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
					bInvalid = true;
				}
			}
			else if (event.code() == aby::Button::KEY_ENTER) {
				if (m_Opts.submit_clears_focus) {
					bFocused = false;
				}
				bInvalid = true;
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
		invalidate_self();

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


namespace aby::ui {

	CursorString::CursorString(std::string& str, size_t init_cursor_pos) :
		m_Buffer(&str),
		m_Cursor(init_cursor_pos),
		m_Highlight(UINT32_MAX, UINT32_MAX)
	{
		if (m_Cursor > m_Buffer->size()) {
			m_Cursor = m_Buffer->size();
		}
	}

	void CursorString::move_right(std::size_t count, bool highlight) {
		std::size_t size = m_Buffer->size();

		if (highlight) {
			m_Highlight.x = m_Cursor;
		}

		while (count != 0 && m_Cursor < size) {
			m_Cursor++;
			count--;
		}

		if (highlight) {
			m_Highlight.y = m_Cursor;
		}

		if (!highlight) {
			m_Highlight.x = UINT32_MAX;
			m_Highlight.y = UINT32_MAX;
		}
	}

	void CursorString::move_left(std::size_t count, bool highlight) {
		if (highlight) {
			m_Highlight.y = m_Cursor;
		}

		while (count != 0 && m_Cursor > 0) {
			m_Cursor--;
			count--;
		}

		if (highlight) {
			m_Highlight.x = m_Cursor;
		}
		if (!highlight) {
			m_Highlight.x = UINT32_MAX;
			m_Highlight.y = UINT32_MAX;
		}
	}
	void CursorString::move_end(bool highlight) {
		move_right(m_Buffer->size() - m_Cursor, highlight);
	}

	void CursorString::move_front(bool highlight) {
		move_left(m_Cursor, highlight);
	}
	void CursorString::move_to(std::size_t position, bool highlight) {
		if (position == m_Cursor) return;
		if (position > m_Cursor) {
			move_right(position - m_Cursor, highlight);
		}
		else {
			move_left(m_Cursor - position, highlight);
		}
	}

	void CursorString::move_next(bool highlight) {
		std::size_t next_space = m_Buffer->find(" ", m_Cursor);
		if (next_space == std::string::npos) {
			move_end(highlight);
		}
		else {
			std::size_t next_word_start = next_space;
			while (next_word_start < m_Buffer->size() && (*m_Buffer)[next_word_start] == ' ') {
				next_word_start++;
			}
			move_right(next_word_start - m_Cursor, highlight);
		}
	}

	void CursorString::move_previous(bool highlight) {
		std::size_t prev_space = m_Buffer->rfind(" ", m_Cursor - 2);
		if (prev_space == std::string::npos) {
			move_front(highlight);
		}
		else {
			std::size_t prev_word_start = prev_space;
			while (prev_word_start > 0 && (*m_Buffer)[prev_word_start - 1] == ' ') {
				prev_word_start--;
			}
			move_left(m_Cursor - prev_word_start - 1, highlight);
		}
	}


	void CursorString::delete_at() {
		m_Cursor = std::clamp(m_Cursor, 0LLU, m_Buffer->size());
		if (m_Cursor > 0) {
			m_Buffer->erase(m_Cursor - 1, 1);
			m_Cursor--;
		}
	}

	void CursorString::insert_at(char character) {
		m_Cursor = std::clamp(m_Cursor, 0LLU, m_Buffer->size());
		m_Buffer->insert(m_Cursor, 1, character);
		m_Cursor++; 
	}

	bool CursorString::is_highlighted() const {
		return (m_Highlight.x != UINT32_MAX && m_Highlight.y != UINT32_MAX) && (m_Highlight.x != m_Highlight.y);
	}

	bool CursorString::is_cursor_at_end() const {
		return m_Cursor == m_Buffer->size();
	}

	std::string CursorString::format() const {
		if (m_Buffer->empty()) {
			return "<ul> </ul>";
		}

		std::string formatted = *m_Buffer;
		std::vector<util::Insertion> ins;

		if (this->is_highlighted()) {
			ins.push_back({ m_Highlight.x, "<hl>" });
			ins.push_back({ m_Highlight.y, "</hl>" });
		}
		if (this->is_cursor_at_end()) {
			formatted.append("<ul> </ul>");
		}
		else {
			ins.push_back({ m_Cursor,     "<ul>"  });
			ins.push_back({ m_Cursor + 1, "</ul>" });
		}

		util::multi_insert(formatted, ins);

		return formatted;
	}

	std::size_t CursorString::position() const {
		return m_Cursor;
	}




}
