#pragma once

#include "Rendering/UI/Button.h"

namespace aby::ui {

	class InputTextbox : public Button {
	public:
		static Ref<InputTextbox> create(const Transform& transform, const Style& style, const TextInfo& info, InputTextOptions opts = {});

		InputTextbox(const Transform& transform, const Style& style, const TextInfo& info, InputTextOptions opts = {});

		void on_event(App* app, Event& event) override;
		void on_pressed() override;
		void on_released() override;
	protected:
		virtual void on_submit(const std::string& msg);
		bool on_key_pressed(KeyPressedEvent& event);
		bool on_key_typed(KeyTypedEvent& event);
		bool on_mouse_released(MouseReleasedEvent& event) override;
	private:
		bool bFocused;
		InputTextOptions m_Opts;
	};

}