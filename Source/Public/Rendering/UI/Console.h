#pragma once

#include "Rendering/UI/LayoutContainer.h"
#include "Rendering/UI/InputTextbox.h"
#include "Rendering/UI/Dropdown.h"
#include "Platform/Process.h"

namespace aby::ui {

	class Console;

	class ConsoleInputTextbox : public InputTextbox {
	public:
		static Ref<ConsoleInputTextbox> create(Console* console, const Style& style);
		ConsoleInputTextbox(Console* console, const Style& style);
		void on_submit(const std::string& msg) override;
	private:
		Console* m_Console;
	};

	class Console : public LayoutContainer {
	public:
		static Ref<Console> create(const Style& style);

		Console(const Style& style);
		~Console() = default;

		void on_create(App* app, bool deserialized) override;
		void on_tick(App* app, Time time) override;
		void on_event(App* app, Event& event) override;
		void on_destroy(App* app) override;
		bool on_invalidate() override;
		void on_resize(EResize direction, float distance) override;

		void add_msg(const LogMsg& msg);
		void for_each(const for_each_fn& fn) override;
		void scroll_down();
		void scroll_up();

		void scroll_to_bottom();
		void exec_cmd(const std::string& cmd);
		void exec_aby_cmd(const std::string& cmd);
		void exec_sys_cmd(const std::string& cmd);
	protected:
		bool on_key_pressed(KeyPressedEvent& event);
		bool on_window_resize(WindowResizeEvent& event) override;
		bool on_mouse_scrolled(MouseScrolledEvent& event);
	protected:
		glm::vec2 calc_item_pos(std::size_t item) const;
		std::uint32_t calc_max_items() const;
		void try_pop();
		Ref<Textbox> create_msg(const LogMsg& msg);
		void add_msg(Ref<Textbox> textbox);
	private:
		struct Constraints {
			// Item height, (logs, input, menu)
			float		item_height = 20.f;
			// Max items on screen, including input and menu.
			std::size_t max_items   = 0;
			// Max logs to be kept in history for scrolling.
			std::size_t max_logs	= 300;
		}; 
		struct Objects {
			Ref<ConsoleInputTextbox> input;
			Ref<LayoutContainer>     menu;
				Ref<Textbox>		 date_time;
				Ref<Textbox>	     fps;
				Ref<Dropdown>        opts;
		};
		Constraints				  m_Constraints;
		std::size_t				  m_Callback;
		std::size_t				  m_ScrollPosition;
		App*					  m_App;
		Objects					  m_Objs;
		Unique<sys::Process>      m_ActiveChannel;
		float					  m_ResizeAcc;
	};

}

