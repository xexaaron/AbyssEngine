#include "Utility/CursorString.h"
#include "Utility/Inserter.h"

namespace aby::util {

	CursorString::CursorString(std::string& str, size_t init_cursor_pos) :
		m_Buffer(&str),
		m_Cursor(init_cursor_pos),
		m_HighlightStart(std::string::npos),
		m_HighlightEnd(std::string::npos)
	{
		if (m_Cursor > m_Buffer->size()) {
			m_Cursor = m_Buffer->size();
		}
	}

	void CursorString::move_right(std::size_t count, bool highlight) {
		std::size_t size = m_Buffer->size();

		if (highlight && !this->is_highlighted()) {
			m_HighlightStart = m_Cursor;
		}

		while (count != 0 && m_Cursor < size) {
			m_Cursor++;
			count--;
		}

		if (highlight) {
			m_HighlightEnd = m_Cursor;
		}

		if (!highlight) {
			this->reset_highlight();
		}
	}

	void CursorString::move_left(std::size_t count, bool highlight) {
		if (highlight && !this->is_highlighted()) {
			m_HighlightEnd = m_Cursor;
		}

		while (count != 0 && m_Cursor > 0) {
			m_Cursor--;
			count--;
		}

		if (highlight) {
			m_HighlightStart = m_Cursor;
		}
		if (!highlight) {
			this->reset_highlight();
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
		if (this->is_highlighted()) {
			m_Buffer->erase(m_Buffer->begin() + m_HighlightStart, m_Buffer->begin() + m_HighlightEnd);
			m_Cursor = m_HighlightStart;
			this->reset_highlight();
		}
		else {
			if (m_Cursor > 0) {
				m_Buffer->erase(m_Cursor - 1, 1);
				m_Cursor--;
			}
		}

	}

	void CursorString::insert_at(char character) {
		m_Cursor = std::clamp(m_Cursor, 0LLU, m_Buffer->size());
		m_Buffer->insert(m_Cursor, 1, character);
		m_Cursor++;
	}

	void CursorString::reset_highlight() {
		m_HighlightStart = std::string::npos;
		m_HighlightEnd = std::string::npos;
	}


	bool CursorString::is_highlighted() const {
		bool out_of_bounds = (m_HighlightStart != std::string::npos && m_HighlightEnd != std::string::npos);
		bool nequal = (m_HighlightStart != m_HighlightEnd);
		return out_of_bounds && nequal;
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
			ins.push_back({ m_HighlightStart, "<hl>" });
			ins.push_back({ m_HighlightEnd, "</hl>" });
		}
		if (this->is_cursor_at_end()) {
			formatted.append("<ul> </ul>");
		}
		else {
			ins.push_back({ m_Cursor,     "<ul>" });
			ins.push_back({ m_Cursor + 1, "</ul>" });
		}

		util::multi_insert(formatted, ins);

		return formatted;
	}

	std::size_t CursorString::position() const {
		return m_Cursor;
	}
}
