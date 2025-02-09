#pragma once
#include "Core/Event.h"
#include "Core/Object.h"
#include <glm/glm.hpp>

namespace aby {
    
    class Camera : public Object {
	public:
		Camera(float fov = 30.f, float aspect_ratio = 1.778f, float near_clip = 0.1f, float far_clip = 1000.f);
		~Camera() = default;

        void on_tick(App* app, Time deltatime) override;
		void on_event(App* app, Event& event) override;

		float distance() const;
		const glm::mat4& projection() const;
		const glm::mat4& view_matrix() const;
		glm::mat4 view_projection() const;
		glm::vec3 up() const;
		glm::vec3 right() const;
		glm::vec3 forward() const;
		const glm::vec3& position() const;
		glm::quat orientation() const;
		float pitch() const;
		float yaw() const;

		void set_distance(float distance);
		void set_viewport(float w, float h);
	private:
		void update_projection();
		void update_view();
		bool on_mouse_scrolled(MouseScrolledEvent& event);
		void mouse_pan(const glm::vec2& delta);
		void mouse_rotate(const glm::vec2& delta);
		void mouse_zoom(float delta);
		glm::vec3 calculate_position() const;
		glm::vec2 pan_speed() const;
		float rotation_speed() const;
		float zoom_speed() const;
	private:
		float      m_FOV = 45.0f;
		float      m_AspectRatio = 1.778f;
		float      m_NearClip = 0.1f;
		float      m_FarClip = 1000.0f;
		float      m_Distance = 10.0f;
		float      m_Yaw = 0.0f;
		float      m_Pitch = 0.0f;
		float      m_ViewportHeight = 1280;
		float      m_ViewportWidth = 720;
		glm::vec2  m_InitialMousePosition;
		glm::vec3  m_Position = { 0.0f, 0.0f, 10.f };
		glm::vec3  m_FocalPoint = { 0.0f, 0.0f, 0.0f };
		glm::mat4  m_ViewMatrix;
		glm::mat4  m_Projection;
		friend class SceneHierarchyPanel;
	};
}