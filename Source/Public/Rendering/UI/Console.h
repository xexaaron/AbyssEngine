#pragma once

#include "Rendering/UI/LayoutContainer.h"
#include "Rendering/UI/InputTextbox.h"
#include <deque>
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

		void on_create(App* app, bool deserialzied) override;
		void on_tick(App* app, Time time) override;
		void on_event(App* app, Event& event) override;
		void on_destroy(App* app) override;
		bool on_invalidate() override;

		void add_msg(const LogMsg& msg);
		void for_each(for_each_fn&& fn) override;
	protected:
		bool on_window_resize(WindowResizeEvent& event) override;
		bool on_mouse_scrolled(MouseScrolledEvent& event);
	protected:
		glm::vec2 calc_item_pos(std::size_t item);
		std::uint32_t calc_max_items();
		void try_pop();
		Ref<Textbox> create_msg(const LogMsg& msg);
		void add_msg(Ref<Textbox> textbox);
	private:
		struct Constraints {
			std::uint32_t max_items   = 0;
			float		  item_height = 0.f;
			std::size_t   max_logs	  = 100;
		}; 
		Constraints				  m_Constraints;
		std::size_t				  m_Callback;
		App*					  m_App;
		Ref<ConsoleInputTextbox>  m_Input;
		Ref<LayoutContainer>	  m_MenuBar;
		std::uint32_t			  m_ScrollPosition;
	private:
		friend class ConsoleInputTextbox;
	};

}

