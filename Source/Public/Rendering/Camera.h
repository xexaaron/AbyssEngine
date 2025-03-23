#pragma once
#include "Core/Event.h"
#include "Core/Object.h"
#include <glm/glm.hpp>

namespace aby {
    

	enum class ECamera {
		DEFAULT  = 0,
		ORIENTED = 0,	// The camera is oriented around a focal point, commonly the world origin.
		FREE     = 1,	// The camera is free to move around.
	};

	class ICamera : public Object {
	public:
		struct Config {
			float FOV;
			float AspectRatio;
			float NearClip;
			float FarClip;

			Config(float fov = 30.f, float aspect_ratio = 1.778f,
				float near_clip = 0.1f, float far_clip = 1000.f) :
				FOV(fov), AspectRatio(aspect_ratio), NearClip(near_clip),
				FarClip(far_clip) {
			}
		};

		virtual ~ICamera() = default;

		void on_tick(App* app, Time time) override;
		void on_event(App* app, Event& event) override;
		virtual void on_zoom(float delta) = 0;
		bool on_mouse_scrolled(MouseScrolledEvent& event);
		virtual void on_rotate(const glm::vec2& delta) = 0;

		void set_rotation_speed(float speed = 0.8f);
		void set_zoom_speed(float speed = 1.0f);
		void set_viewport(const glm::vec2& viewport_size);

		virtual ECamera type() const = 0;

		virtual glm::vec3 up() const;
		virtual glm::vec3 right() const;
		virtual glm::vec3 forward() const;
		float pitch() const;
		float yaw() const;
		float aspect_ratio() const;
		float fov() const;
		float near_clip() const;
		float far_clip() const;
		float rotation_speed() const;
		float zoom_speed() const;
		const glm::vec2& viewport_size() const;
		const glm::vec3& position() const;
		glm::mat4 view_projection() const;
		glm::quat orientation() const;
	protected:
		explicit ICamera(const Config& cfg = {}, const glm::vec3& pos = glm::vec3(0.f));
		void update_projection();
		virtual void update_view() = 0;
	protected:
		float	  m_Pitch;
		float     m_Yaw;
		float	  m_AspectRatio;
		float	  m_FOV;
		float	  m_NearClip;
		float	  m_FarClip;
		float	  m_RotationSpeed;
		float	  m_ZoomSpeed;
		glm::vec2 m_InitialMousePosition;
		glm::vec2 m_ViewportSize;
		glm::vec3 m_Position;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_Projection;
	};

	class OrientedCamera final : public ICamera {
	public:
		explicit OrientedCamera(const Config& cfg = {}, const glm::vec3& focal_point = glm::vec3(0.f), float distance = 200.f);
		~OrientedCamera() = default;

		void on_tick(App* app, Time deltatime) override;
		void on_zoom(float delta) override;
		void on_rotate(const glm::vec2& delta) override;
		void on_pan(const glm::vec2& delta);

		void set_distance(float distance);
		void set_focal_point(const glm::vec3& focal_point);

		ECamera type() const override;
		float distance() const;
		const glm::vec3& focal_point() const;
	protected:
		void update_view() override;
	private:
		float m_Distance;
		glm::vec3 m_FocalPoint;
	};

	class FreeCamera final : public ICamera {
	public:
		explicit FreeCamera(const Config& cfg = {});

		~FreeCamera() = default;

		void on_tick(App* app, Time time) override;
		void on_zoom(float delta) override;
		void on_rotate(const glm::vec2& delta) override;

		ECamera type() const override;
	protected:
		void update_view() override;
	};

}