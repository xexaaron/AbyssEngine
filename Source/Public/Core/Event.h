#pragma once

#include <iostream>
#include <sstream>
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
			INPUT = (1 << 0),
			KEYBOARD = (1 << 1),
			MOUSE = (1 << 2),
			MOUSE_BUTTON = (1 << 3),
			WINDOW = (1 << 4),
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
	};

	class Event {
	public:
		virtual ~Event() = default;
		virtual EEventType type() const = 0;
		virtual const char* name() const = 0;
		virtual int category() const = 0;
		virtual std::string to_string() const = 0;

		bool is_handled() const { 
			return bHandled;
		}
		bool is_in_category(EEventCategory::E cat) const {
			return category() & cat;
		}

		friend std::ostream& operator<<(std::ostream& os, const Event& ev) {
			return os << ev.to_string();
		}
	private:
		friend class EventDispatcher;
		bool bHandled = false;
	};

	class EventDispatcher {
	public:
		template <typename T>
		using Callback = std::function<bool(T&)>;
	public:
		EventDispatcher(Event& event) : 
			m_DispatcherEvent(event) {}

		template <typename T> requires (std::is_base_of_v<Event, T>)
		bool bind(Callback<T> function) {
			if (m_DispatcherEvent.type() == T::static_type()) {
				m_DispatcherEvent.bHandled = function(*(T*)&m_DispatcherEvent);
				return true;
			}
			return false;
		}

		template <typename C, typename T> requires (std::is_base_of_v<Event, T>)
		bool bind(C* obj, bool(C::* member_fn)(T&)) {
			if (m_DispatcherEvent.type() == T::static_type()) {
				// Call the member function on the object
				m_DispatcherEvent.bHandled = (obj->*member_fn)(*(T*)&m_DispatcherEvent);
				return true;
			}
			return false;
		}


	private:
		Event& m_DispatcherEvent;
	};

	class KeyEvent : public Event {
	public:
		int code() const { 
			return m_KeyCode;
		}

		virtual EEventType type() const override = 0;

		virtual int category() const override {
			return EEventCategory::KEYBOARD | EEventCategory::INPUT;
		};
	protected:
		KeyEvent(Button::EKey key_code) : 
			m_KeyCode(key_code) {}
		Button::EKey m_KeyCode;
	};
	
	class KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(Button::EKey key_code, int repeat_count) :
			KeyEvent(key_code),
			m_RepeatCount(repeat_count)
		{
		}

		int repeat_count() const { 
			return m_RepeatCount;
		}
		std::string to_string() const override {
			std::stringstream ss;
			ss << "Key Pressed Event : " << m_KeyCode << "(Occurences : " << m_RepeatCount << ")";
			return ss.str();
		}
		constexpr static EEventType static_type() { 
			return EEventType::KEY_PRESSED;
		}
		virtual EEventType type() const override { 
			return static_type();
		}
		virtual const char* name() const override { 
			return "KEY_PRESSED";
		}
	private:
		int m_RepeatCount;
	};

	class KeyReleasedEvent : public KeyEvent {
	public:
		KeyReleasedEvent(Button::EKey key_code) :
			KeyEvent(key_code) {}

		std::string to_string() const override {
			std::stringstream ss;
			ss << "Key Released Event : " << m_KeyCode;
			return ss.str();
		}

		constexpr static EEventType static_type() {
			return EEventType::KEY_RELEASED;
		}
		virtual EEventType type() const override {
			return static_type();
		}
		virtual const char* name() const override {
			return "KEY_RELEASED";
		}
	private:
	};

	class KeyTypedEvent : public KeyEvent {
	public:
		KeyTypedEvent(Button::EKey key_code) :
			KeyEvent(key_code) {}

		std::string to_string() const override {
			std::stringstream ss;
			ss << "Key Typed Event : " << m_KeyCode;
			return ss.str();
		}

		constexpr static EEventType static_type() {
			return EEventType::KEY_TYPED;
		}
		virtual EEventType type() const override {
			return static_type();
		}
		virtual const char* name() const override {
			return "KEY_TYPED";
		}
	};

	class MouseMovedEvent : public Event {
	public:
		MouseMovedEvent(float x, float y) : 
			m_Pos(x, y) {}

		float x() const { 
			return m_Pos.x;
		}
		float y() const { 
			return m_Pos.y;
		}
		bool hit(const glm::vec2& pos, const glm::vec2& size) const {
			return (m_Pos.x >= pos.x && m_Pos.x <= pos.x + size.x) &&
				(m_Pos.y >= pos.y && m_Pos.y <= pos.y + size.y);
		}

		glm::vec2 pos() const { 
			return m_Pos;
		}
		std::string to_string() const override {
			std::stringstream ss;
			ss << "Mouse Moved Event : Position(x : " << m_Pos.x << ", y : " << m_Pos.y << ")";
			return ss.str();
		}
		constexpr static EEventType static_type() {
			return EEventType::MOUSE_MOVE;
		}
		virtual EEventType type() const override {
			return static_type();
		}
		virtual const char* name() const override {
			return "MOUSE_MOVED";
		}
		virtual int category() const override {
			return EEventCategory::MOUSE | EEventCategory::INPUT;
		}
	private:
		glm::vec2 m_Pos;
	};

	class MouseScrolledEvent : public Event {
	public:
		MouseScrolledEvent(float offset_x, float offset_y) : 
			m_Offset(offset_x, offset_y) {}

		float offset_x() const { 
			return m_Offset.x;
		}
		float offset_y() const { 
			return m_Offset.y;
		}
		glm::vec2 offset() const { 
			return m_Offset;
		}
		std::string to_string() const override {
			std::stringstream ss;
			ss << "Mouse Scrolled Event : " << "Offset(y: " << m_Offset.x << ", x : " << m_Offset.y << ")";
			return ss.str();
		}

		constexpr static EEventType static_type() {
			return EEventType::MOUSE_SCROLLED;
		}
		virtual EEventType type() const override {
			return static_type();
		}
		virtual const char* name() const override {
			return "MOUSE_SCROLLED";
		}
		virtual int category() const override {
			return EEventCategory::MOUSE | EEventCategory::INPUT;
		}
	private:
		glm::vec2 m_Offset;
	};

	class MouseButtonEvent : public Event {
	public:
		inline int code() const { 
			return m_MouseButton;
		}
		virtual EEventType type() const override = 0;
		virtual int category() const override {
			return EEventCategory::MOUSE | EEventCategory::INPUT;
		}
		bool hit(const glm::vec2& pos, const glm::vec2& size) const {
			return (m_Pos.x >= pos.x && m_Pos.x <= pos.x + size.x) &&
				(m_Pos.y >= pos.y && m_Pos.y <= pos.y + size.y);
		}

		const glm::vec2& pos() const {
			return m_Pos;
		}
	protected:
		MouseButtonEvent(Button::EMouse button, const glm::vec2& pos) :
			m_MouseButton(button),
			m_Pos(pos) {}

		Button::EMouse m_MouseButton;
		glm::vec2 m_Pos;
	};

	class MousePressedEvent : public MouseButtonEvent {
	public:
		MousePressedEvent(Button::EMouse button, const glm::vec2& pos) :
			MouseButtonEvent(button, pos) {}

		std::string to_string() const override {
			std::stringstream ss;
			ss << "Mouse Pressed Event : Button : " << m_MouseButton;
			return ss.str();
		}
		constexpr static EEventType static_type() {
			return EEventType::MOUSE_PRESSED;
		}
		virtual EEventType type() const override {
			return static_type();
		}
		virtual const char* name() const override {
			return "MOUSE_PRESSED";
		}
	private:
	};

	class MouseReleasedEvent : public MouseButtonEvent {
	public:
		MouseReleasedEvent(Button::EMouse button, const glm::vec2& pos) :
			MouseButtonEvent(button, pos) {}

		std::string to_string() const override {
			std::stringstream ss;
			ss << "Mouse Released Event : Button : " << m_MouseButton;
			return ss.str();
		}

		constexpr static EEventType static_type() {
			return EEventType::MOUSE_RELEASED;
		}
		virtual EEventType type() const override {
			return static_type();
		}
		virtual const char* name() const override {
			return "MOUSE_RELEASED";
		}

	private:
	};

	class WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(std::uint32_t w, std::uint32_t h, std::uint32_t old_w, std::uint32_t old_h) :
			m_NewSize(w, h), m_OldSize(old_w, old_h) {}

		std::uint32_t w() const {
			return m_NewSize.x;
		}
		std::uint32_t h() const { 
			return m_NewSize.y;
		}
		glm::u32vec2 size() const {
			return m_NewSize;
		}

		glm::u32vec2 old_size() const {
			return m_OldSize;
		}

		std::string to_string() const override {
			std::stringstream ss;
			ss << "Window Resize Event : (w : " << m_NewSize.x << ", h: " << m_NewSize.y << ")";
			return ss.str();
		}

		constexpr static EEventType static_type() {
			return EEventType::WINDOW_RESIZE;
		}
		virtual EEventType type() const override {
			return static_type();
		}
		virtual const char* name() const override {
			return "WINDOW_RESIZE";
		}
		virtual int category() const override {
			return EEventCategory::WINDOW;
		}

	private:
		glm::u32vec2 m_NewSize;
		glm::u32vec2 m_OldSize;
	};

	class WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() = default;

		std::string to_string() const override {
			std::stringstream ss;
			ss << "Window Close Event";
			return ss.str();
		}

		constexpr static EEventType static_type() {
			return EEventType::WINDOW_CLOSE;
		}
		virtual EEventType type() const override {
			return static_type();
		}
		virtual const char* name() const override {
			return "WINDOW_CLOSE";
		}
		virtual int category() const override {
			return EEventCategory::WINDOW;
		}
	};
}