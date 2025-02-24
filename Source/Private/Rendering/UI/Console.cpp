#include "Rendering/UI/Console.h"

namespace aby::ui::helper {

	glm::vec4 log_color_to_vec4(ELogColor color) {
		switch (color) {
			using enum ELogColor;
			case Yellow: return { 1.0f, 1.0f, 0.0f, 1.0f };  // RGB(255, 255, 0)
			case Cyan:   return { 0.0f, 1.0f, 1.0f, 1.0f };  // RGB(0, 255, 255)
			case Grey:   return { 0.7f, 0.7f, 0.7f, 1.0f };  // RGB(128, 128, 128)
			case Red:    return { 1.0f, 0.0f, 0.0f, 1.0f };  // RGB(255, 0, 0)
			default:     return { 1.0f, 1.0f, 1.0f, 1.0f };  // Default to white
		}
	}

}

namespace aby::ui {


	Ref<Console> Console::create(const Style& style) {
		return create_ref<Console>(style);
	}


	Console::Console(const Style& style) :
		LayoutContainer({ .anchor = {.position = EAnchor::BOTTOM_LEFT, .offset = {}} }, style, EDirection::VERTICAL, ELayout::BOTTOM_TO_TOP),
		m_Constraints{
			.max_items = 0,
			.item_height = 20,
			.max_logs = 100,
		},
		m_Callback(SIZE_MAX),
		m_App(nullptr),
		m_Input(ConsoleInputTextbox::create(this, style)),
		m_MenuBar(LayoutContainer::create({}, style, EDirection::HORIZONTAL, ELayout::AUTO, 2.f)),
		m_ScrollPosition(0)
	{
		
	}

	void Console::on_create(App* app, bool deserialzied) {
		m_Transform.size.x = app->window()->size().x;
		m_Transform.size.y = (app->window()->size().y / 2.5) - m_Constraints.item_height;
		m_Input->set_position({ 0, app->window()->size().y - m_Constraints.item_height });
		m_Input->set_size({ app->window()->size().x, m_Constraints.item_height });
		m_Input->on_create(app, false);
		m_MenuBar->set_position(m_Transform.position);
		m_MenuBar->set_size({ app->window()->size().x, m_Constraints.item_height + 4 });
		m_MenuBar->add_child(Button::create(
			Transform{ .anchor = {}, .position = { 0, 0 }, .size = { 100, m_Constraints.item_height } },
			ButtonStyle::dark_mode(),
			TextInfo{
				.text = "Options",
				.color = { 1, 1, 1, 1 },
				.scale = 1.f,
				.alignment = ETextAlignment::CENTER
			},
			false
		));
		m_MenuBar->on_create(app, false);
		m_Constraints.max_items = calc_max_items();
		m_App = app;
		LayoutContainer::on_create(app, deserialzied);
		invalidate_self();
		m_Callback = Logger::add_callback([this](LogMsg msg) {
			this->add_msg(msg);
		});
	}

	bool Console::on_window_resize(WindowResizeEvent& event) {
		m_Transform.size.x	   = event.size().x;
		m_Transform.size.y     = (event.size().y / 2.5) - m_Constraints.item_height;
		m_Input->set_position({ 0, event.size().y - m_Constraints.item_height });
		m_Input->set_size({ event.size().x, m_Constraints.item_height });
		m_MenuBar->set_position(m_Transform.position);
		m_MenuBar->set_size({ event.size().x, m_Constraints.item_height + 4 });
		invalidate_self();
		return false;
	}

	std::uint32_t Console::calc_max_items() {
		return std::max(0, static_cast<int>(std::floorf((m_Transform.size.y - (m_Constraints.item_height * 2)) / m_Constraints.item_height)));
	}

	void Console::for_each(for_each_fn&& fn) {
		if (m_Children.empty()) {
			return;
		}

		// Define the visible range based on scroll position
		std::uint32_t start = std::max(0u, m_ScrollPosition);
		std::uint32_t end   = std::min(start + m_Constraints.max_items, static_cast<std::uint32_t>(m_Children.size()));

		// Iterate in reverse from `end - 1` down to `start`
		for (std::int32_t i = static_cast<std::int32_t>(end) - 1; i >= static_cast<std::int32_t>(start); i--) {
			fn(m_Children[i]);
		}
	}

	bool Console::on_mouse_scrolled(MouseScrolledEvent& event) {
		if (m_Children.size() > m_Constraints.max_items) {
			if (event.offset_y() > 0) { // Scroll up
				if (m_ScrollPosition > 0) {
					m_ScrollPosition--;
				}
				LayoutContainer::for_each([this](Ref<Widget> widget, std::size_t i) {
					widget->set_position(calc_item_pos(i));
					widget->on_invalidate();
				});
			}
			else if (event.offset_y() < 0) { // Scroll down
				if (m_ScrollPosition < static_cast<int>(m_Children.size()) - m_Constraints.max_items) {
					m_ScrollPosition++;
				}
				LayoutContainer::for_each([this](Ref<Widget> widget, std::size_t i) {
					widget->set_position(calc_item_pos(i));
					widget->on_invalidate();
				});
			}

		}
		invalidate_self();
		return false;
	}

	bool Console::on_invalidate() {
		m_Constraints.max_items = calc_max_items();
		m_Input->on_invalidate();
		m_MenuBar->set_invalid(true);
		m_MenuBar->on_invalidate();
		for (auto& child : m_MenuBar->children()) {
			child->on_invalidate();
		}
		return LayoutContainer::on_invalidate();
	}

	void Console::on_tick(App* app, Time time) {
		if (!bVisible) return;
		LayoutContainer::on_tick(app, time);
		m_Input->on_tick(app, time);
		m_MenuBar->on_tick(app, time);
	}

	void Console::on_event(App* app, Event& event) {
		if (!bVisible) return;
		EventDispatcher dsp(event);
		dsp.bind(this, &Console::on_mouse_scrolled);
		m_Input->on_event(app, event);
		m_MenuBar->on_event(app, event);
		LayoutContainer::on_event(app, event);
	}

	void Console::on_destroy(App* app)  {
		LayoutContainer::on_destroy(app);
		m_Input->on_destroy(app);
		m_MenuBar->on_destroy(app);
		Logger::remove_callback(m_Callback);
	}

	glm::vec2 Console::calc_item_pos(std::size_t item) {
		glm::vec2 pos = {
			0,
			((m_Transform.position.y + m_Transform.size.y)) - ((item + 2) * (m_Constraints.item_height))
		};
		pos.y = std::clamp(
			pos.y,
			m_Transform.position.y + m_Constraints.item_height,
			m_Transform.position.y + m_Transform.size.y - m_Constraints.item_height
		);
		return pos;
	}

	void Console::try_pop() {
		while (m_Children.size() > m_Constraints.max_logs) {
			remove_child(0);
		}
	}

	void Console::add_msg(const LogMsg& msg) {
		try_pop();
		auto textbox = create_msg(msg);
		textbox->on_create(m_App, false);
		add_child(std::move(textbox));
		LayoutContainer::for_each([this](Ref<Widget> widget, std::size_t i) {
			widget->set_position(calc_item_pos(i));
			widget->on_invalidate();
		});
		invalidate_self();
	}

	void Console::add_msg(Ref<Textbox> textbox) {
		try_pop();
		textbox->on_create(m_App, false);
		add_child(std::move(textbox));
		LayoutContainer::for_each([this](Ref<Widget> widget, std::size_t i) {
			widget->set_position(calc_item_pos(i));
			widget->on_invalidate();
		});
		invalidate_self();
	}

	Ref<Textbox> Console::create_msg(const LogMsg& msg) {
		std::size_t index = std::min(m_Children.size(), static_cast<std::size_t>(m_Constraints.max_items));
		Transform transform{
			.anchor = {},
			.position = calc_item_pos(index), 
			.size = { m_Transform.size.x, m_Constraints.item_height }
		};

		TextInfo info{
			.text = "Child: (" + std::to_string(m_Children.size()) + ") " + msg.text,
			.color = helper::log_color_to_vec4(msg.color()),
			.scale = 1.f,
			.alignment = ETextAlignment::LEFT
		};

		static Style style{
			.background = m_Style.background,
			.border = {},
		};

		return Textbox::create(transform, style, info);
	}
}

namespace aby::ui {
	Ref<ConsoleInputTextbox> ConsoleInputTextbox::create(Console* console, const Style& style) {
		return create_ref<ConsoleInputTextbox>(console, style);
	}

	ConsoleInputTextbox::ConsoleInputTextbox(Console* console, const Style& style) :
		InputTextbox(
			Transform{},
			style,
			TextInfo{
				.text = ">>> ",
				.color = { 1, 1, 1, 1 },
				.scale = 1.f,
				.alignment = ETextAlignment::LEFT,
			},
			InputTextOptions{
				.prefix = 4,
				.submit_clears_focus = false,
				.submit_clears_text = true,
			}
		),
		m_Console(console)
	{

	}

	void ConsoleInputTextbox::on_submit(const std::string& msg) {
		ABY_LOG("{}", msg);
		m_Console->m_ScrollPosition = m_Console->children().size() - m_Console->m_Constraints.max_items + 1;
	}
}
