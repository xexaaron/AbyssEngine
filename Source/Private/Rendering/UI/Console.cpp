#include "Rendering/UI/Console.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef open
#undef open
#endif

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
			.max_logs = 300,
		},
		m_Callback(SIZE_MAX),
		m_App(nullptr),
		m_Objs{
			.input = ConsoleInputTextbox::create(this, style),
			.menu  = LayoutContainer::create({}, style, EDirection::HORIZONTAL, ELayout::AUTO, 2.f),
			.date_time = nullptr,
			.fps = nullptr,
			.opts = Dropdown::create(
				Transform{ {}, {}, {100, m_Constraints.item_height}},
				ButtonStyle{
					.hovered  = { m_Style.background.color * 1.1f, Resource{} },
					.pressed  = m_Style.background,
					.released = m_Style.background,
					.border   = {}
				},
				TextInfo{ "Options", { 1, 1, 1, 1 }, 1.f, ETextAlignment::CENTER },
				{ 100, 100 }
			)
		},
		m_ScrollPosition(0),
		m_ActiveChannel(nullptr)
	{
		Transform t{
			.anchor = {},
			.position = { 0, 0 },
			.size = { 150, m_Constraints.item_height }
		};
		Style s{
			.background = Style::dark_mode().background,
			.border = {}
		};
		TextInfo ti{
			.text  = Logger::time_date_now(),
			.color = { 0.759, 0.761, 0.754, 1 },
			.scale = 1.f,
			.alignment = ETextAlignment::CENTER
		};
		m_Objs.date_time = Textbox::create(t, s, ti);
		t.size.x = 80;
		ti.text = "60 FPS";
		m_Objs.fps = Textbox::create(t, s, ti);
	}

	void Console::on_create(App* app, bool deserialzied) {
		m_Transform.size.x = app->window()->size().x;
		m_Transform.size.y = (app->window()->size().y / 2.5) - m_Constraints.item_height;
		m_Objs.input->set_position({ 0, app->window()->size().y - m_Constraints.item_height });
		m_Objs.input->set_size({ app->window()->size().x, m_Constraints.item_height });
		m_Objs.input->on_create(app, false);
		m_Objs.opts->add_child(Button::create(
			Transform{ {}, {}, { m_Objs.opts->transform().size.x, m_Constraints.item_height } },
			m_Objs.opts->button_style(),
			TextInfo{
				.text = "Checkmark",
				.color = { 1, 1, 1, 1 },
				.scale = 1.f,
				.alignment = ETextAlignment::LEFT
			},
			false
		));
		m_Objs.menu->set_position(m_Transform.position);
		m_Objs.menu->set_size({ app->window()->size().x, m_Constraints.item_height + 4 });
		m_Objs.menu->add_child(m_Objs.date_time);
		m_Objs.menu->add_child(m_Objs.fps);
		m_Objs.menu->add_child(m_Objs.opts);
		m_Objs.menu->on_create(app, false);
		m_Constraints.max_items = calc_max_items();
		m_App = app;
		LayoutContainer::on_create(app, deserialzied);
		invalidate_self();
		m_Callback = Logger::add_callback([this](LogMsg msg) {
			this->add_msg(msg);
			this->scroll_to_bottom();
		});
	}

	bool Console::on_window_resize(WindowResizeEvent& event) {
		m_Transform.size.x	   = event.size().x;
		m_Transform.size.y     = (event.size().y / 2.5) - m_Constraints.item_height;
		m_Constraints.max_items = calc_max_items();
		m_Objs.input->set_position({ 0, event.size().y - m_Constraints.item_height });
		m_Objs.input->set_size({ event.size().x, m_Constraints.item_height });
		m_Objs.menu->set_position(m_Transform.position);
		m_Objs.menu->set_size({ event.size().x, m_Constraints.item_height + 4 });
		invalidate_self();
		return false;
	}

	std::uint32_t Console::calc_max_items() {
		return std::max(0, static_cast<int>((m_Transform.size.y - (m_Constraints.item_height * 2)) / m_Constraints.item_height));
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
		constexpr auto scroll_amt = 3;
		if (m_Children.size() > m_Constraints.max_items) {
			if (event.offset_y() > 0) { // Scroll up
				for (std::size_t i = 0; i < scroll_amt; i++) {
					if (m_ScrollPosition > 0) {
						m_ScrollPosition--;
					}
					else {
						break;
					}
				}
				LayoutContainer::for_each([this](Ref<Widget> widget, std::size_t i) {
					widget->set_position(calc_item_pos(i));
					widget->on_invalidate();
				});
			}
			else if (event.offset_y() < 0) { // Scroll down
				for (std::size_t i = 0; i < scroll_amt; i++) {
					if (m_ScrollPosition < static_cast<int>(m_Children.size()) - m_Constraints.max_items) {
						m_ScrollPosition++;
					}
					else {
						break;
					}
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
		m_Objs.input->on_invalidate();
		m_Objs.menu->set_invalid(true);
		m_Objs.menu->on_invalidate();
		for (auto& child : m_Objs.menu->children()) {
			child->on_invalidate();
		}
		return LayoutContainer::on_invalidate();
	}

	void Console::on_tick(App* app, Time time) {
		if (!bVisible) return;

		LayoutContainer::on_tick(app, time);
		m_Objs.input->on_tick(app, time);
		m_Objs.menu->on_tick(app, time);

		static float elapsed = 0.f;
		elapsed += time.sec();
		if (elapsed >= 1.f) {
			int fps = (time.sec() > 0.0f) ? (1.f / time.sec()) : 0.0f;
			m_Objs.fps->set_text(std::format("{} FPS", fps));
			m_Objs.date_time->set_text(Logger::time_date_now());
			elapsed -= 1.f; 
		}
	}

	void Console::on_event(App* app, Event& event) {
		if (!bVisible) return;
		EventDispatcher dsp(event);
		dsp.bind(this, &Console::on_mouse_scrolled);
		dsp.bind(this, &Console::on_key_pressed);
		m_Objs.input->on_event(app, event);
		m_Objs.menu->on_event(app, event);
		LayoutContainer::on_event(app, event);
	}

	void Console::on_destroy(App* app)  {
		LayoutContainer::on_destroy(app);
		m_Objs.input->on_destroy(app);
		m_Objs.menu->on_destroy(app);
		if (m_ActiveChannel && m_ActiveChannel->is_open()) {
			m_ActiveChannel->close();
		}
		Logger::remove_callback(m_Callback);
	}

	bool Console::on_key_pressed(KeyPressedEvent& event) {
		if ((event.mods() & aby::Button::EMod::CTRL) != aby::Button::EMod::NONE &&
			event.code() == aby::Button::KEY_C) 
		{
			if (m_ActiveChannel) {
				m_ActiveChannel->kill();
				m_ActiveChannel.reset();
				return true;
			}
		}

		return false;
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
		if (msg.text.starts_with("[Info] >>>")) {
			return;
		}
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
			.text  = msg.text,
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

	void Console::exec_cmd(const std::string& cmd) {
		if (cmd.starts_with("app:")) {
			exec_aby_cmd(cmd.substr(4));
		}
		else {
			exec_sys_cmd(cmd);
		}
		ABY_LOG("{}", cmd);
	}


	void Console::exec_aby_cmd(const std::string& cmd) {
		if (cmd == "quit") {
			m_App->quit();
		}
		else if (cmd == "restart") {
			m_App->restart();
		}
	}

	void Console::exec_sys_cmd(const std::string& cmd) {
		if (m_ActiveChannel && m_ActiveChannel->is_open()) {
			m_ActiveChannel->write(cmd); // Send command
		}
		else {
			m_ActiveChannel = sys::Process::create([](const std::string& msg) {
				ABY_LOG("{}", msg);
			});
			m_ActiveChannel->open(cmd);
		}
	}

	void Console::scroll_to_bottom() {
		if (m_Children.size() > m_Constraints.max_items) {
			m_ScrollPosition = m_Children.size() - m_Constraints.max_items + 1;
		}
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
				.color = { 0.759, 0.761, 0.754, 1 },
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
		m_Console->exec_cmd(msg);
		m_Console->scroll_to_bottom();
	}
}
