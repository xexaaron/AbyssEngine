#pragma once

#include "Widget/Widget.h"
#include "Widget/LayoutContainer.h"
#include "Widget/Button.h"

namespace aby::ui {

	class WidgetSwitcher : public Widget {
	public:
		static Ref<WidgetSwitcher> create(const Transform& transform, const ImageStyle& tab_style);

		WidgetSwitcher(const Transform& transform, const ImageStyle& tab_style);
		

		void on_create(App* app, bool) override;
		void on_tick(App* app, Time deltatime) override;

		void for_each(std::function<void(Ref<Widget>)> fn) override;
		std::size_t add_child(Ref<Widget> child) override;

		Ref<Widget> get_active();

		void set_active(std::size_t index);
	protected:
		Ref<LayoutContainer> get_tabs();
	private:
		std::size_t m_ActiveWidget;
		glm::vec2 m_TabSize;
		ImageStyle m_TabStyle;
	};

}