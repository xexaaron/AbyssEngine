#pragma once

#include "Rendering/UI/Button.h"

namespace aby::ui {


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

		bool is_cursor_at_end() const;
		bool is_highlighted() const;
		std::string format() const;
		std::size_t position() const;
	private:
        std::string* m_Buffer;
        std::size_t  m_Cursor;
		glm::u32vec2 m_Highlight;
    };

	class InputTextbox : public Button {
	public:
		static Ref<InputTextbox> create(const Transform& transform, const Style& style, const TextInfo& info, InputTextOptions opts = {});

		InputTextbox(const Transform& transform, const Style& style, const TextInfo& info, InputTextOptions opts = {});

		void on_tick(App* app, Time deltatime) override;
		void on_event(App* app, Event& event) override;
		void on_pressed() override;
		void on_released() override;

		void set_options(const InputTextOptions& options);
		void set_focused(bool focused);
		void set_cursor_pos(std::size_t position);
	protected:
		virtual void on_submit(const std::string& msg);
		bool on_key_pressed(KeyPressedEvent& event);
		bool on_key_typed(KeyTypedEvent& event);
		bool on_mouse_released(MouseReleasedEvent& event) override;
	private:
		bool bFocused;
		InputTextOptions m_Opts;
		CursorString     m_Cursor;
	};

}