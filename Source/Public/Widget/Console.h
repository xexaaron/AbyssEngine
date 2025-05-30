#pragma once

#include "Widget/LayoutContainer.h"
#include "Widget/InputTextbox.h"
#include "Widget/Dropdown.h"
#include "Platform/Process.h"

namespace aby::ui {

	class Console;

	class ConsoleInputTextbox : public InputTextbox {
	public:
		static Ref<ConsoleInputTextbox> create(Console* console, const ImageStyle& style);
		ConsoleInputTextbox(Console* console, const ImageStyle& style);
		void on_submit(const std::string& msg) override;
	private:
		Console* m_Console;
	};

	class Console : public LayoutContainer {
	public:
		static Ref<Console> create(const ImageStyle& style);

		Console(const ImageStyle& style);
		~Console() = default;

		void on_create(App* app, bool deserialized) override;
		void on_tick(App* app, Time time) override;
		void on_event(App* app, Event& event) override;
		void on_destroy(App* app) override;
		void on_resize(EResize direction, float distance) override;

		void add_msg(const LogMsg& msg);
		void scroll_down();
		void scroll_up();

		void scroll_to_bottom();
		void exec_cmd(const std::string& cmd);
		void exec_aby_cmd(const std::string& cmd);
		void exec_sys_cmd(const std::string& cmd);

		void for_each(std::function<void(Ref<Widget>)> fn) override;

		float item_height() const;
	protected:
		bool on_key_pressed(KeyPressedEvent& event);
		bool on_window_resize(WindowResizeEvent& event) override;
		bool on_mouse_scrolled(MouseScrolledEvent& event);
		bool on_mouse_released(MouseReleasedEvent& event);
		bool on_mouse_moved(MouseMovedEvent& event);
	protected:
		glm::vec2 calc_item_pos(std::size_t item) const;
		u32 calc_max_items() const;
		void try_pop();
		Ref<Textbox> create_msg(const LogMsg& msg);
		void add_msg(Ref<Textbox> textbox);
	private:
		struct Constraints {
			float        item_height = 20.f; // Item height, (logs, input, menu)
			std::size_t  max_items   = 0;    // Max items on screen, including input and menu.
			std::size_t  max_logs	= 300;   // Max logs to be kept in history for scrolling.
		}; 
		struct Objects {
			Ref<ConsoleInputTextbox> input;
			Ref<LayoutContainer>     menu;
				Ref<Textbox>		 date_time;
				Ref<Textbox>	     fps;
				Ref<Dropdown>        opts;
		};
		struct State {
			std::size_t			 scroll_pos;	 // Offset into m_Children.
			float 				 resize_acc;	 // Handle resize amt.
			bool				 focused;		 // Allow for typing when focused.
			bool				 hovered;		 // Allow for scrolling when hovered.
			Unique<sys::Process> active_channel; // Process to send to or start when typing command
		};
		struct Config {
			std::size_t  callback;
			i32 scroll_amt;
		};
		App*		m_App;
		Constraints	m_Constraints;
		State		m_State;
		Objects		m_Objs;
		Config		m_Cfg;
	};

}