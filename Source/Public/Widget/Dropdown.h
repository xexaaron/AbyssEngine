#pragma once

#include "Widget/Button.h"
#include "Widget/LayoutContainer.h"

namespace aby::ui {
	
	class Dropdown : public ui::Button {
	public:
		static Ref<Dropdown> create(const Transform& button_transform, const ButtonStyle& style, const TextInfo& text_info, const glm::vec2& dropdown_size);

		Dropdown(const Transform& button_transform, const ButtonStyle& style, const TextInfo& text_info, const glm::vec2& dropdown_size);

		void on_create(App* app, bool deserialized) override;
		void on_tick(App* app, Time deltatime) override;
		void on_event(App* app, Event& event) override;
		void on_released() override;
		void on_unhovered() override;
	protected:
		bool on_window_resize(WindowResizeEvent& event) override;
	private:
		Ref<LayoutContainer> m_Container;
		glm::vec2 m_DropdownSize;
	};


}