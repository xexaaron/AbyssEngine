#include "Rendering/UI/Dropdown.h"

namespace aby::ui {
	
	Ref<Dropdown> Dropdown::create(const Transform& button_transform, const ButtonStyle& style, const TextInfo& text_info, const glm::vec2& dropdown_size) {
		return create_ref<Dropdown>(button_transform, style, text_info, dropdown_size);
	}

	Dropdown::Dropdown(const Transform& transform, const ButtonStyle& style, const TextInfo& text_info, const glm::vec2& dropdown_size) :
		ui::Button(transform, style, text_info, false),
		m_Container(LayoutContainer::create(
			Transform{ 
				.anchor   = {},
				.position = { m_Transform.position.x - 2.f, m_Transform.position.y + m_Transform.size.y },
				.size	  = dropdown_size
			}, 
			ImageStyle{
				style.border,
				style.released.color,
				style.released.texture
			}, 
			EDirection::VERTICAL, 
			ELayout::TOP_TO_BOTTOM,
			2.f)
		),
		m_DropdownSize(dropdown_size)
	{
		m_Name = "Dropdown";
	}

	void Dropdown::on_create(App* app, bool deserialized) {
		ui::Button::on_create(app, deserialized);
		m_Container->on_create(app, false);
	}
	
	void Dropdown::on_tick(App* app, Time deltatime) {
		ui::Button::on_tick(app, deltatime);
		if (m_Container->is_visible()) {
			auto mouse = m_Window->mouse_pos();
			const auto& t = m_Container->transform();
			const auto& t1 = m_Transform;
			if (!(mouse.x >= t.position.x && mouse.x <= (t.position.x + t.size.x) &&
				  mouse.y >= t.position.y && mouse.y <= (t.position.y + t.size.y)) &&
				!(mouse.x >= t1.position.x && mouse.x <= (t1.position.x + t1.size.x) && 
				  mouse.y >= t1.position.y && mouse.y <= (t1.position.y + t1.size.y))) 
			{
				m_Container->set_visible(false);
			}
		}
		m_Container->on_tick(app, deltatime);
	}
	
	void Dropdown::on_event(App* app, Event& event) {
		ui::Button::on_event(app, event);
		m_Container->on_event(app, event);
	}
	
	void Dropdown::on_released() {
		m_Container->set_visible(!m_Container->is_visible());
	}

	void Dropdown::on_unhovered() {
		auto mouse = m_Window->mouse_pos();
		const auto& t = m_Container->transform();
		const auto& t1 = m_Transform;
		if (!(mouse.x >= t.position.x  && mouse.x <= (t.position.x +  t.size.x)  &&
			  mouse.y >= t.position.y  && mouse.y <= (t.position.y +  t.size.y)) &&
			!(mouse.x >= t1.position.x && mouse.x <= (t1.position.x + t1.size.x) &&
			  mouse.y >= t1.position.y && mouse.y <= (t1.position.y + t1.size.y)))
		{
			m_Container->set_visible(false);
		}
	}

	bool Dropdown::on_window_resize(WindowResizeEvent& event) {
		bool res = ui::Button::on_window_resize(event);
		return res;
	}
}