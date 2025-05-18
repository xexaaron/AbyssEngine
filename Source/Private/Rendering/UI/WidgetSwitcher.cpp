#include "Rendering/UI/WidgetSwitcher.h"

namespace aby::ui {

	WidgetSwitcher::WidgetSwitcher(const Transform& transform, const ImageStyle& tab_style) :
		Widget(transform, "WidgetSwitcher"),
		m_ActiveWidget(1),
		m_TabSize(30, 20)
	{
		Transform tab_transform = transform;
		tab_transform.size.y = m_TabSize.y;
		tab_transform.anchor.position = EAnchor::TOP_LEFT;
		tab_transform.anchor.offset = { 0, 0 };
		auto container = LayoutContainer::create(tab_transform, tab_style, EDirection::HORIZONTAL);
		Super::add_child(container);
	}
	
	Ref<WidgetSwitcher> WidgetSwitcher::create(const Transform& transform, const ImageStyle& tab_style) {
		return create_ref<WidgetSwitcher>(transform, tab_style);
	}

	std::size_t WidgetSwitcher::add_child(Ref<Widget> child) {
		std::size_t idx = Super::add_child(child);
		auto tabs = get_tabs();
		Transform tab_tf;
		tab_tf.size = m_TabSize;
		TextInfo text_info;
		text_info.alignment = ETextAlignment::CENTER;
		text_info.color     = { 1.f, 1.f, 1.f, 1.f };
		text_info.scale		= 1.f;
		text_info.text		= child->name();
		Ref<Button> tab = Button::create(tab_tf, ButtonStyle::dark_mode(), text_info, false);
		tabs->add_child(tab);
		auto child_pos = m_Transform.position;
		child_pos.y -= m_TabSize.y;
		child->set_position(child_pos);
		return idx;
	}
	void WidgetSwitcher::on_tick(App* app, Time deltatime) {
		Super::on_tick(app, deltatime);
	}
	void WidgetSwitcher::for_each(std::function<void(Ref<Widget>)> fn) {
		ABY_ASSERT(m_ActiveWidget > m_Children.size(), "Out of bounds");
		fn(m_Children[0]);
		fn(m_Children[m_ActiveWidget]);
	}

	Ref<Widget> WidgetSwitcher::get_active() {
		ABY_ASSERT(m_ActiveWidget < m_Children.size(), "Out of bounds");
		return m_Children[m_ActiveWidget];
	}

	Ref<LayoutContainer> WidgetSwitcher::get_tabs() {
		ABY_ASSERT(!m_Children.empty(), "Out of bounds");
		return std::static_pointer_cast<LayoutContainer>(m_Children[0]);
	}

	void WidgetSwitcher::set_active(std::size_t index) {
		m_ActiveWidget = std::clamp(index, std::size_t(1), m_Children.size() - 1);
	}

}