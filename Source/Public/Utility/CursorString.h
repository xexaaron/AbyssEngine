#include "Core/Common.h"

namespace aby::util {

	class CursorString {
	public:
		CursorString(std::string& str, size_t init_cursor_pos = 0);

		void move_right(std::size_t count = 1, bool highlight = false);
		void move_left(std::size_t count = 1, bool highlight = false);
		void move_to(std::size_t position, bool highlight = false);
		void move_next(bool highlight = false);
		void move_previous(bool highlight = false);
		void move_end(bool highlight = false);
		void move_front(bool highlight = false);

		void delete_at();
		void insert_at(char character);
		void reset_highlight();

		bool is_cursor_at_end() const;
		bool is_highlighted() const;
		std::string format() const;
		std::size_t position() const;
	private:
		std::string* m_Buffer;
		std::size_t  m_Cursor;
		std::size_t  m_HighlightStart;
		std::size_t  m_HighlightEnd;
	};

}