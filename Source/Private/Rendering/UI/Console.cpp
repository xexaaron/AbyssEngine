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

namespace aby::ui {
	Ref<ConsoleInputTextbox> ConsoleInputTextbox::create(Console* console, const ImageStyle& style) {
		return create_ref<ConsoleInputTextbox>(console, style);
	}

	ConsoleInputTextbox::ConsoleInputTextbox(Console* console, const ImageStyle& style) :
		InputTextbox(
			Transform{},
			style,
			TextInfo{
				.text = "",
				.color = { 0.759, 0.761, 0.754, 1 },
				.scale = 1.f,
				.alignment = ETextAlignment::LEFT,
			},
			InputTextOptions{
				.submit_clears_focus = false,
				.submit_clears_text  = true,
				.cursor				 = true, 
			}
			),
		m_Console(console)
	{
		m_Text.prefix = ">>> ";
		m_Name = "ConsoleInputTextbox";
	}

	void ConsoleInputTextbox::on_submit(const std::string& msg) {
		m_Console->exec_cmd(msg);
		m_Console->scroll_to_bottom();
	}

}

namespace aby::ui {

	Ref<Console> Console::create(const ImageStyle& style) {
		return create_ref<Console>(style);
	}

	Console::Console(const ImageStyle& style) :
		LayoutContainer({ .anchor = {.position = EAnchor::BOTTOM_LEFT, .offset = {}} }, style, EDirection::VERTICAL, ELayout::BOTTOM_TO_TOP),
		m_Constraints{
			.item_height = 20,
			.max_items = 0,
			.max_logs = 300,
		},
		m_App(nullptr),
		m_Objs{
			.input = ConsoleInputTextbox::create(this, style),
			.menu  = LayoutContainer::create({}, style, EDirection::HORIZONTAL, ELayout::AUTO, 2.f),
			.date_time = nullptr,
			.fps = nullptr,
			.opts = Dropdown::create(
				Transform{ {}, {}, {100, m_Constraints.item_height}},
				ButtonStyle{
					.hovered  = { m_Background * 1.1f, Resource{} },
					.pressed  = m_Background,
					.released = m_Background,
					.border   = {}
				},
				TextInfo{ "Options", { 1, 1, 1, 1 }, 1.f, ETextAlignment::CENTER },
				{ 100, 100 }
			)
		},
		m_State{
			.scroll_pos		= 0,
			.resize_acc		= 0.f,
			.focused		= false,
			.active_channel = nullptr,
		},
		m_Cfg{
			.callback   = SIZE_MAX,
			.scroll_amt = 3,
		}
	{
		Transform transform{
			.anchor = {},
			.position = { 0, 0 },
			.size = { 150, m_Constraints.item_height }
		};
		ImageStyle obj_style{
			.border  = {},
			.color   = ButtonStyle::dark_mode().released.color,
		};
		TextInfo text_info{
			.text  = Logger::time_date_now(),
			.color = { 0.759, 0.761, 0.754, 1 },
			.scale = 1.f,
			.alignment = ETextAlignment::CENTER
		};
		m_Objs.date_time = Textbox::create(transform, obj_style, TextInfo{ text_info });
		transform.size.x = 80;
		text_info.text = "60 FPS";
		m_Objs.fps = Textbox::create(transform, obj_style, text_info);
		m_Name = "Console";
		set_resizability(EResize::N);
	}

	void Console::on_create(App* app, bool deserialzied) {
		auto x = static_cast<float>(app->window()->size().x);
		auto y = static_cast<float>(app->window()->size().y);
		m_Transform.size.x = x;
		m_Transform.size.y = (y / 2.5f) - m_Constraints.item_height;
		m_Objs.input->set_position({ 0, y - m_Constraints.item_height });
		m_Objs.input->set_size({ x, m_Constraints.item_height });
		m_Objs.input->on_create(app, false);
		// m_Objs.opts->children().push_back(Button::create(
		// 	Transform{ {}, {}, { m_Objs.opts->transform().size.x, m_Constraints.item_height } },
		// 	m_Objs.opts->button_style(),
		// 	TextInfo{
		// 		.text = "Checkmark",
		// 		.color = { 1, 1, 1, 1 },
		// 		.scale = 1.f,
		// 		.alignment = ETextAlignment::LEFT
		// 	},
		// 	false
		// ));
		m_Objs.menu->set_position(m_Transform.position);
		m_Objs.menu->set_size({ x, m_Constraints.item_height + 4 });
		m_Objs.menu->add_child(m_Objs.date_time);
		m_Objs.menu->add_child(m_Objs.fps);
		m_Objs.menu->add_child(m_Objs.opts);
		m_Objs.menu->on_create(app, false);
		m_Constraints.max_items = calc_max_items();
		m_App = app;
		LayoutContainer::on_create(app, deserialzied);
		m_Cfg.callback = Logger::add_callback([this](const LogMsg& msg) {
			this->add_msg(msg);
			this->scroll_to_bottom();
		});
	}

	bool Console::on_window_resize(WindowResizeEvent& event) {
		auto x = static_cast<float>(event.size().x);
		auto y = static_cast<float>(event.size().y);
		m_Transform.size.x	   = x;
		m_Constraints.max_items = calc_max_items();
		m_Objs.input->set_position({ 0, y - m_Constraints.item_height });
		m_Objs.input->set_size({ x, m_Constraints.item_height });
		m_Objs.menu->set_position(m_Transform.position);
		m_Objs.menu->set_size({ x, m_Constraints.item_height + 4 });
		return false;
	}

	void Console::on_resize(EResize direction, float distance) {
		// m_Objs.menu->set_position(m_Transform.position);
		// m_Constraints.max_items = calc_max_items();
		// m_ResizeAcc += std::abs(distance);
		// if (m_ResizeAcc >= m_Constraints.item_height)
		// {
		// 	m_ResizeAcc = 0;
		// 	scroll_up();
		// 	LayoutContainer::for_each([this](Ref<Widget> widget, std::size_t i) {
		// 		widget->set_position(calc_item_pos(i));
		// 		widget->on_invalidate();
		// 	});
		// 	invalidate_self();
		// }
	}

	bool Console::on_mouse_released(MouseReleasedEvent& event) {
		m_State.focused = m_State.hovered;
		if (!m_State.focused) { return false; }

		auto input_transform = m_Objs.input->transform();
		bool input_focused   = event.hit(input_transform.position, input_transform.size);
		m_Objs.input->set_focused(input_focused);
		
		if (input_focused)
			if (auto pos = m_Objs.input->hit_text(event.pos()); pos != std::string::npos)
				m_Objs.input->set_cursor_pos(pos);
		
		return false;
	}

	bool Console::on_mouse_moved(MouseMovedEvent& event) {
		m_State.hovered = event.hit(m_Transform.position, m_Transform.size);
		return false;
	}

	void Console::on_tick(App* app, Time time) {
		if (!bVisible) return;

		LayoutContainer::on_tick(app, time);
		m_Objs.input->on_tick(app, time);
		m_Objs.menu->on_tick(app, time);
		static float elapsed = 0.f;
		elapsed += time.sec();
		if (elapsed >= 1.f) {
			int fps = static_cast<int>((time.sec() > 0.0f) ? (1.f / time.sec()) : 0.0f);
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
		dsp.bind(this, &Console::on_mouse_released);
		dsp.bind(this, &Console::on_mouse_moved);
		m_Objs.input->on_event(app, event);
		m_Objs.menu->on_event(app, event);
		LayoutContainer::on_event(app, event);

	}

	void Console::on_destroy(App* app)  {
		LayoutContainer::on_destroy(app);
		m_Objs.input->on_destroy(app);
		m_Objs.menu->on_destroy(app);
		if (m_State.active_channel && m_State.active_channel->is_open()) {
			m_State.active_channel->close();
		}
		Logger::remove_callback(m_Cfg.callback);
	}

	bool Console::on_key_pressed(KeyPressedEvent& event) {
		if ((event.mods() & ::aby::Button::EMod::CTRL) != ::aby::Button::EMod::NONE &&
			event.code() == ::aby::Button::KEY_C) 
		{
			if (m_State.active_channel) {
				m_State.active_channel->kill();
				m_State.active_channel.reset();
				return true;
			}
		}

		return false;
	}

	bool Console::on_mouse_scrolled(MouseScrolledEvent& event) {
		if (m_State.hovered) {
			if (m_Children.size() > m_Constraints.max_items) {
				if (event.offset_y() > 0) { // Scroll up
					scroll_up();
				}
				else if (event.offset_y() < 0) { // Scroll down
					scroll_down();
				}
				Widget::for_each([this](Ref<Widget> widget, std::size_t i) {
					widget->set_position(calc_item_pos(i));
				});
			}
		}
		return false;
	}






	void Console::try_pop() {
		while (m_Children.size() > m_Constraints.max_logs) {
			m_Children.erase(m_Children.begin());
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
		Widget::for_each([this](Ref<Widget> widget, std::size_t i) {
			widget->set_position(calc_item_pos(i));
		});
	}

	void Console::add_msg(Ref<Textbox> textbox) {
		try_pop();
		textbox->on_create(m_App, false);
		add_child(std::move(textbox));
		Widget::for_each([this](Ref<Widget> widget, std::size_t i) {
			widget->set_position(calc_item_pos(i));
		});
	}

	Ref<Textbox> Console::create_msg(const LogMsg& msg) {
		std::size_t index = std::min(m_Children.size(), static_cast<std::size_t>(m_Constraints.max_items));
		Transform transform{
			.anchor   = {},
			.position = calc_item_pos(index), 
			.size     = { m_Transform.size.x, m_Constraints.item_height }
		};

		TextInfo info{
			.text      = msg.text,
			.color     = Logger::log_color_to_vec4(msg.color()),
			.scale	   = 1.f,
			.alignment = ETextAlignment::LEFT
		};

		ImageStyle style{
			.border = {},
			.color= m_Background,
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
		if (m_State.active_channel && m_State.active_channel->is_open()) {
			m_State.active_channel->write(cmd);
		}
		else {
			auto read = [](const std::string& msg) {
				ABY_LOG("{}", msg);
			};
			m_State.active_channel = sys::Process::create(read);
			m_State.active_channel->open(cmd);
		}
	}
	
	glm::vec2 Console::calc_item_pos(std::size_t item) const {
		// Initial position of item index 0:
		// m_Transform.position.y + m_Transform.size.y
		// Subtract:
		//		The index of the item add 1 * the height per item.
		// Offsets it by the space that console input item takes.
		// Clamp y 
		// by the position + the height per item offsetting it by the space that the console
		// menubar takes up.
		// and 
		// The full size of the console - the height per item, the space that the console input
		// item takes.
		// However the issue lies in that affecting the offset
		// Doesnt actually seem to change anything in the positions of items.
		// We are always positioning our max_items below 2 item heights ending at the 
		// bottom of the console
		float offset = 1;
		glm::vec2 pos = {
			0,
			(m_Transform.position.y + m_Transform.size.y) - ((item + offset) * (m_Constraints.item_height)) // Shifted by 1 item_height
		};
		// pos.y = std::clamp(
		// 	pos.y,
		// 	m_Transform.position.y + (offset * m_Constraints.item_height),
		// 	m_Transform.position.y + m_Transform.size.y - (offset * m_Constraints.item_height)
		// );
		return pos;
	}

	u32 Console::calc_max_items() const {
		// Subtract the space taken by the input box and menu area
		float avail_height = m_Transform.size.y - m_Constraints.item_height * 2;
		return std::max(0, static_cast<int>(avail_height / m_Constraints.item_height));
	}

	void Console::for_each(std::function<void(Ref<Widget>)> fn) {
		if (m_Children.empty()) {
			return;
		}

		std::size_t start = std::max(std::size_t(0), m_State.scroll_pos);
		std::size_t end = std::min(start + m_Constraints.max_items, m_Children.size());
		for (std::ptrdiff_t i = static_cast<std::ptrdiff_t>(end) - 1; i >= static_cast<std::ptrdiff_t>(start); i--) {
			fn(m_Children[i]);
		}
	}


	void Console::scroll_to_bottom() {
		if (m_Children.size() > m_Constraints.max_items) {
			m_State.scroll_pos = m_Children.size() - m_Constraints.max_items;
		}
	}

	void Console::scroll_down() {
		for (std::size_t i = 0; i < m_Cfg.scroll_amt; i++) {
			if (m_State.scroll_pos < m_Children.size() - m_Constraints.max_items) {
				m_State.scroll_pos++;
			}
			else {
				break;
			}
		}
	}

	void Console::scroll_up() {
		for (std::size_t i = 0; i < m_Cfg.scroll_amt; i++) {
			if (m_State.scroll_pos > 0) {
				m_State.scroll_pos--;
			} else {
				break;
			}
		}

	}



}
