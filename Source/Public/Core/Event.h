#pragma once

#include "Core/Common.h"
#include <functional>
#include <glm/glm.hpp>

namespace aby {

	enum class EEventType {
		NONE = 0,
		WINDOW_CLOSE,
		WINDOW_RESIZE,
		WINDOW_FOCUS,
		WINDOW_LOST_FOCUS,
		WINDOW_MOVED,
		KEY_PRESSED,
		KEY_RELEASED,
		KEY_TYPED,
		MOUSE_PRESSED,
		MOUSE_RELEASED,
		MOUSE_MOVE,
		MOUSE_SCROLLED,
	};

	struct EEventCategory {
		enum E {
			NONE = 0,
			INPUT		 = BIT(0),
			KEYBOARD	 = BIT(1),
			MOUSE		 = BIT(2),
			MOUSE_BUTTON = BIT(3),
			WINDOW		 = BIT(4),
		};
	};

	struct Button {
		enum EKey : int {
			KEY_SPACE = 32,
			KEY_APOSTROPHE = 39,/* ' */
			KEY_COMMA = 44,/* , */
			KEY_MINUS = 45,/* - */
			KEY_PERIOD = 46,/* . */
			KEY_SLASH = 47,/* / */
			KEY_0 = 48,
			KEY_1 = 49,
			KEY_2 = 50,
			KEY_3 = 51,
			KEY_4 = 52,
			KEY_5 = 53,
			KEY_6 = 54,
			KEY_7 = 55,
			KEY_8 = 56,
			KEY_9 = 57,
			KEY_SEMICOLON = 59,/* ; */
			KEY_EQUAL = 61,/* = */
			KEY_A = 65,
			KEY_B = 66,
			KEY_C = 67,
			KEY_D = 68,
			KEY_E = 69,
			KEY_F = 70,
			KEY_G = 71,
			KEY_H = 72,
			KEY_I = 73,
			KEY_J = 74,
			KEY_K = 75,
			KEY_L = 76,
			KEY_M = 77,
			KEY_N = 78,
			KEY_O = 79,
			KEY_P = 80,
			KEY_Q = 81,
			KEY_R = 82,
			KEY_S = 83,
			KEY_T = 84,
			KEY_U = 85,
			KEY_V = 86,
			KEY_W = 87,
			KEY_X = 88,
			KEY_Y = 89,
			KEY_Z = 90,
			KEY_LEFT_BRACKET = 91,/* [ */
			KEY_BACKSLASH = 92,/* \ */
			KEY_RIGHT_BRACKET = 93,/* ] */
			KEY_GRAVE_ACCENT = 96,/* ` */
			KEY_WORLD_1 = 161,/* non-US #1 */
			KEY_WORLD_2 = 162,/* non-US #2 */
			KEY_ESCAPE = 256,/* Function keys */
			KEY_ENTER = 257,
			KEY_TAB = 258,
			KEY_BACKSPACE = 259,
			KEY_INSERT = 260,
			KEY_DELETE = 261,
			KEY_ARROW_RIGHT = 262,
			KEY_ARROW_LEFT = 263,
			KEY_ARROW_DOWN = 264,
			KEY_ARROW_UP = 265,
			KEY_PAGE_UP = 266,
			KEY_PAGE_DOWN = 267,
			KEY_HOME = 268,
			KEY_END = 269,
			KEY_CAPS_LOCK = 280,
			KEY_SCROLL_LOCK = 281,
			KEY_NUM_LOCK = 282,
			KEY_PRINT_SCREEN = 283,
			KEY_PAUSE = 284,
			KEY_F1 = 290,
			KEY_F2 = 291,
			KEY_F3 = 292,
			KEY_F4 = 293,
			KEY_F5 = 294,
			KEY_F6 = 295,
			KEY_F7 = 296,
			KEY_F8 = 297,
			KEY_F9 = 298,
			KEY_F10 = 299,
			KEY_F11 = 300,
			KEY_F12 = 301,
			KEY_F13 = 302,
			KEY_F14 = 303,
			KEY_F15 = 304,
			KEY_F16 = 305,
			KEY_F17 = 306,
			KEY_F18 = 307,
			KEY_F19 = 308,
			KEY_F20 = 309,
			KEY_F21 = 310,
			KEY_F22 = 311,
			KEY_F23 = 312,
			KEY_F24 = 313,
			KEY_F25 = 314,
			KEY_KP_0 = 320,
			KEY_KP_1 = 321,
			KEY_KP_2 = 322,
			KEY_KP_3 = 323,
			KEY_KP_4 = 324,
			KEY_KP_5 = 325,
			KEY_KP_6 = 326,
			KEY_KP_7 = 327,
			KEY_KP_8 = 328,
			KEY_KP_9 = 329,
			KEY_KP_DECIMAL = 330,
			KEY_KP_DIVIDE = 331,
			KEY_KP_MULTIPLY = 332,
			KEY_KP_SUBTRACT = 333,
			KEY_KP_ADD = 334,
			KEY_KP_ENTER = 335,
			KEY_KP_EQUAL = 336,
			KEY_LEFT_SHIFT = 340,
			KEY_LEFT_CONTROL = 341,
			KEY_LEFT_ALT = 342,
			KEY_LEFT_SUPER = 343,
			KEY_LEFT_WIN = KEY_LEFT_SUPER,
			KEY_LEFT_CMD = KEY_LEFT_SUPER,
			KEY_RIGHT_SHIFT = 344,
			KEY_RIGHT_CONTROL = 345,
			KEY_RIGHT_ALT = 346,
			KEY_RIGHT_SUPER = 347,
			KEY_RIGHT_WIN = KEY_RIGHT_SUPER,
			KEY_RIGHT_CMD = KEY_RIGHT_SUPER,
			KEY_MENU = 348,
			KEY_LAST = KEY_MENU,
		};

		enum EMouse : int {
			MOUSE_1 = 0,
			MOUSE_2 = 1,
			MOUSE_3 = 2,
			MOUSE_4 = 3,
			MOUSE_5 = 4,
			MOUSE_6 = 5,
			MOUSE_7 = 6,
			MOUSE_8 = 7,
			MOUSE_LAST = MOUSE_8,
			MOUSE_LEFT = MOUSE_1,
			MOUSE_RIGHT = MOUSE_2,
			MOUSE_MIDDLE = MOUSE_3,
		};
	
		enum class EMod {
			NONE  = 0,
			SHIFT = BIT(0),
			CTRL  = BIT(1),
			ALT	  = BIT(2),
			SUPER = BIT(3),
			CAPS  = BIT(4),
			NUML  = BIT(5)
		};
	};
	DECLARE_ENUM_OPS(Button::EMod);

	class Event {
	public:
		virtual ~Event() = default;
		virtual EEventType type() const = 0;
		virtual const char* name() const = 0;
		virtual int category() const = 0;
		virtual std::string to_string() const = 0;

		bool is_handled() const;
		bool is_in_category(EEventCategory::E cat) const;

		friend std::ostream& operator<<(std::ostream& os, const Event& ev);
	private:
		friend class EventDispatcher;
		bool bHandled = false;
	};

	class EventDispatcher {
	public:
		template <typename T>
		using Callback = std::function<bool(T&)>;
	public:
		explicit EventDispatcher(Event& event) :
			m_DispatcherEvent(event) {}

		template <typename T> requires (std::is_base_of_v<Event, T>)
		bool bind(Callback<T> function) {
			if (m_DispatcherEvent.type() == T::static_type()) {
				m_DispatcherEvent.bHandled = function(static_cast<T&>(m_DispatcherEvent));
				return true;
			}
			return false;
		}

		template <typename C, typename T> requires (std::is_base_of_v<Event, T>)
		bool bind(C* obj, bool(C::* member_fn)(T&)) {
			if (m_DispatcherEvent.type() == T::static_type()) {
				// Call the member function on the object
				m_DispatcherEvent.bHandled = (obj->*member_fn)(static_cast<T&>(m_DispatcherEvent));
				return true;
			}
			return false;
		}
	private:
		Event& m_DispatcherEvent;
	};

	class KeyEvent : public Event {
	public:
		int code() const;
		EEventType type() const override = 0;
		int category() const override;
	protected:
		explicit KeyEvent(Button::EKey key_code);
		Button::EKey m_KeyCode;
	};
	
	class KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(Button::EKey key_code, int repeat_count, Button::EMod mods);

		int repeat_count() const;
		Button::EMod mods() const;
		std::string to_string() const override;
		EEventType type() const override;
		const char* name() const override;
		constexpr static EEventType static_type() { return EEventType::KEY_PRESSED; }
	private:
		int m_RepeatCount;
		Button::EMod m_Mods;
	};

	class KeyReleasedEvent : public KeyEvent {
	public:
		explicit KeyReleasedEvent(Button::EKey key_code);

		std::string to_string() const override;
		EEventType type() const override;
		const char* name() const override;
		constexpr static EEventType static_type() { return EEventType::KEY_RELEASED; }
	private:
	};

	class KeyTypedEvent : public KeyEvent {
	public:
		explicit KeyTypedEvent(Button::EKey key_code);

		std::string to_string() const override;
		EEventType type() const override;
		const char* name() const override;

		constexpr static EEventType static_type() { return EEventType::KEY_TYPED; }
	};

	class MouseMovedEvent : public Event {
	public:
		MouseMovedEvent(float x, float y);

		float x() const;
		float y() const;
		bool hit(const glm::vec2& pos, const glm::vec2& size) const;

		glm::vec2 pos() const;
		std::string to_string() const override;
	
		EEventType type() const override;
		const char* name() const override;
		int category() const override;

		constexpr static EEventType static_type() { return EEventType::MOUSE_MOVE; }
	private:
		glm::vec2 m_Pos;
	};

	class MouseScrolledEvent : public Event {
	public:
		MouseScrolledEvent(float offset_x, float offset_y);

		float offset_x() const;
		float offset_y() const;
		glm::vec2 offset() const;
		std::string to_string() const override;
	
		EEventType type() const override;
		const char* name() const override;
		int category() const override;
		constexpr static EEventType static_type() { return EEventType::MOUSE_SCROLLED; }
	private:
		glm::vec2 m_Offset;
	};

	class MouseButtonEvent : public Event {
	public:
		int code() const;
		EEventType type() const override = 0;
		int category() const override;
		bool hit(const glm::vec2& pos, const glm::vec2& size) const;
		const glm::vec2& pos() const;
	protected:
		MouseButtonEvent(Button::EMouse button, const glm::vec2& pos);

		Button::EMouse m_MouseButton;
		glm::vec2 m_Pos;
	};

	class MousePressedEvent : public MouseButtonEvent {
	public:
		MousePressedEvent(Button::EMouse button, const glm::vec2& pos);

		std::string to_string() const override;

		EEventType type() const override;
		const char* name() const override;

		constexpr static EEventType static_type() { return EEventType::MOUSE_PRESSED; }
	};

	class MouseReleasedEvent : public MouseButtonEvent {
	public:
		MouseReleasedEvent(Button::EMouse button, const glm::vec2& pos);

		std::string to_string() const override;
		EEventType type() const override;
		const char* name() const override;
		constexpr static EEventType static_type() { return EEventType::MOUSE_RELEASED; }
	};

	class WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(u32 w, u32 h, u32 old_w, u32 old_h);

		u32 w() const;
		u32 h() const;
		glm::u32vec2 size() const;
		glm::u32vec2 old_size() const;
		std::string to_string() const override;
		EEventType type() const override;
		const char* name() const override;
		int category() const override;
		constexpr static EEventType static_type() { return EEventType::WINDOW_RESIZE; }
	private:
		glm::u32vec2 m_NewSize;
		glm::u32vec2 m_OldSize;
	};

	class WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() = default;

		std::string to_string() const override;
		EEventType type() const override;
		const char* name() const override;
		int category() const override;
		constexpr static EEventType static_type() { return EEventType::WINDOW_CLOSE; }
	};
}