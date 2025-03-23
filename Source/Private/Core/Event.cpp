#include "Core/Event.h"

namespace aby {
	bool Event::is_handled() const {
		return bHandled;
	}
	bool Event::is_in_category(EEventCategory::E cat) const {
		return category() & cat;
	}
	std::ostream& operator<<(std::ostream& os, const Event& ev) {
		return os << ev.to_string();
	}
	int KeyEvent::code() const {
		return m_KeyCode;
	}
	int KeyEvent::category() const {
		return EEventCategory::KEYBOARD | EEventCategory::INPUT;
	}
	KeyEvent::KeyEvent(Button::EKey key_code) :
		m_KeyCode(key_code) {
	}
	KeyPressedEvent::KeyPressedEvent(Button::EKey key_code, int repeat_count, Button::EMod mods) :
		KeyEvent(key_code),
		m_RepeatCount(repeat_count),
		m_Mods(mods)
	{
	}
	int KeyPressedEvent::repeat_count() const {
		return m_RepeatCount;
	}
	std::string KeyPressedEvent::to_string() const {
		std::stringstream ss;
		ss << "Key Pressed Event : " << m_KeyCode << "(Occurrences : " << m_RepeatCount << ")";
		return ss.str();
	}
	
	EEventType KeyPressedEvent::type() const {
		return static_type();
	}
	const char* KeyPressedEvent::name() const {
		return "KEY_PRESSED";
	}
	int MouseButtonEvent::code() const {
		return m_MouseButton;
	}
	int MouseButtonEvent::category() const {
		return EEventCategory::MOUSE | EEventCategory::INPUT;
	}
	bool MouseButtonEvent::hit(const glm::vec2& pos, const glm::vec2& size) const {
		return (m_Pos.x >= pos.x && m_Pos.x <= pos.x + size.x) &&
			(m_Pos.y >= pos.y && m_Pos.y <= pos.y + size.y);
	}
	const glm::vec2& MouseButtonEvent::pos() const {
		return m_Pos;
	}
	MouseButtonEvent::MouseButtonEvent(Button::EMouse button, const glm::vec2& pos) :
		m_MouseButton(button),
		m_Pos(pos) {
	}
	MousePressedEvent::MousePressedEvent(Button::EMouse button, const glm::vec2& pos) :
		MouseButtonEvent(button, pos) {
	}
	std::string MousePressedEvent::to_string() const {
		std::stringstream ss;
		ss << "Mouse Pressed Event : Button : " << m_MouseButton;
		return ss.str();
	}
	
	EEventType MousePressedEvent::type() const {
		return static_type();
	}
	const char* MousePressedEvent::name() const {
		return "MOUSE_PRESSED";
	}
	MouseReleasedEvent::MouseReleasedEvent(Button::EMouse button, const glm::vec2& pos) :
		MouseButtonEvent(button, pos) {
	}
	std::string MouseReleasedEvent::to_string() const {
		std::stringstream ss;
		ss << "Mouse Released Event : Button : " << m_MouseButton;
		return ss.str();
	}
	
	EEventType MouseReleasedEvent::type() const {
		return static_type();
	}
	const char* MouseReleasedEvent::name() const {
		return "MOUSE_RELEASED";
	}
	WindowResizeEvent::WindowResizeEvent(std::uint32_t w, std::uint32_t h, std::uint32_t old_w, std::uint32_t old_h) :
		m_NewSize(w, h), m_OldSize(old_w, old_h) {
	}
	std::uint32_t WindowResizeEvent::w() const {
		return m_NewSize.x;
	}
	std::uint32_t WindowResizeEvent::h() const {
		return m_NewSize.y;
	}
	glm::u32vec2 WindowResizeEvent::size() const {
		return m_NewSize;
	}
	glm::u32vec2 WindowResizeEvent::old_size() const {
		return m_OldSize;
	}
	std::string WindowResizeEvent::to_string() const {
		std::stringstream ss;
		ss << "Window Resize Event : (w : " << m_NewSize.x << ", h: " << m_NewSize.y << ")";
		return ss.str();
	}

	EEventType WindowResizeEvent::type() const {
		return static_type();
	}
	const char* WindowResizeEvent::name() const {
		return "WINDOW_RESIZE";
	}
	int WindowResizeEvent::category() const {
		return EEventCategory::WINDOW;
	}
	std::string WindowCloseEvent::to_string() const {
		std::stringstream ss;
		ss << "Window Close Event";
		return ss.str();
	}
	
	EEventType WindowCloseEvent::type() const {
		return static_type();
	}
	const char* WindowCloseEvent::name() const {
		return "WINDOW_CLOSE";
	}
	int WindowCloseEvent::category() const {
		return EEventCategory::WINDOW;
	}
	KeyReleasedEvent::KeyReleasedEvent(Button::EKey key_code) :
		KeyEvent(key_code) {
	}
	std::string KeyReleasedEvent::to_string() const {
		std::stringstream ss;
		ss << "Key Released Event : " << m_KeyCode;
		return ss.str();
	}
	
	EEventType KeyReleasedEvent::type() const {
		return static_type();
	}
	const char* KeyReleasedEvent::name() const {
		return "KEY_RELEASED";
	}
	KeyTypedEvent::KeyTypedEvent(Button::EKey key_code) :
		KeyEvent(key_code) {
	}
	std::string KeyTypedEvent::to_string() const {
		std::stringstream ss;
		ss << "Key Typed Event : " << m_KeyCode;
		return ss.str();
	}
	EEventType KeyTypedEvent::type() const {
		return static_type();
	}
	const char* KeyTypedEvent::name() const {
		return "KEY_TYPED";
	}
	MouseMovedEvent::MouseMovedEvent(float x, float y) :
		m_Pos(x, y) {
	}
	float MouseMovedEvent::x() const {
		return m_Pos.x;
	}
	float MouseMovedEvent::y() const {
		return m_Pos.y;
	}
	bool MouseMovedEvent::hit(const glm::vec2& pos, const glm::vec2& size) const {
		return (m_Pos.x >= pos.x && m_Pos.x <= pos.x + size.x) &&
			(m_Pos.y >= pos.y && m_Pos.y <= pos.y + size.y);
	}
	glm::vec2 MouseMovedEvent::pos() const {
		return m_Pos;
	}
	std::string MouseMovedEvent::to_string() const {
		std::stringstream ss;
		ss << "Mouse Moved Event : Position(x : " << m_Pos.x << ", y : " << m_Pos.y << ")";
		return ss.str();
	}
	EEventType MouseMovedEvent::type() const {
		return static_type();
	}
	const char* MouseMovedEvent::name() const {
		return "MOUSE_MOVED";
	}
	int MouseMovedEvent::category() const {
		return EEventCategory::MOUSE | EEventCategory::INPUT;
	}
	MouseScrolledEvent::MouseScrolledEvent(float offset_x, float offset_y) :
		m_Offset(offset_x, offset_y) {
	}
	float MouseScrolledEvent::offset_x() const {
		return m_Offset.x;
	}
	float MouseScrolledEvent::offset_y() const {
		return m_Offset.y;
	}
	glm::vec2 MouseScrolledEvent::offset() const {
		return m_Offset;
	}
	std::string MouseScrolledEvent::to_string() const {
		std::stringstream ss;
		ss << "Mouse Scrolled Event : " << "Offset(y: " << m_Offset.x << ", x : " << m_Offset.y << ")";
		return ss.str();
	}
	EEventType MouseScrolledEvent::type() const {
		return static_type();
	}
	const char* MouseScrolledEvent::name() const {
		return "MOUSE_SCROLLED";
	}
	int MouseScrolledEvent::category() const {
		return EEventCategory::MOUSE | EEventCategory::INPUT;
	}

	Button::EMod KeyPressedEvent::mods() const {
		return m_Mods;
	}

}