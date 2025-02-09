#include "Rendering/Camera.h"
#include "Core/App.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace aby {

    Camera::Camera(float fov, float aspect_ratio, float near_clip, float far_clip) :
        m_FOV(fov),
        m_AspectRatio(aspect_ratio),
        m_NearClip(near_clip),
        m_FarClip(far_clip),
        m_Distance(200),
        m_Projection(glm::perspective(glm::radians(fov), aspect_ratio, near_clip, far_clip)) 
    {
        update_view();
    }

    void Camera::on_tick(App* app, Time  deltatime) {
        update_view();
        auto window = app->window();
        if (window->is_key_pressed(Button::KEY_LEFT_ALT)) {
            auto mouse_pos = window->mouse_pos();
            glm::vec2 delta = (mouse_pos - m_InitialMousePosition) * 0.003f;
            m_InitialMousePosition = mouse_pos;
            if (window->is_mouse_pressed(Button::MOUSE_MIDDLE)) {
                mouse_pan(delta);
            } 
            if (window->is_mouse_pressed(Button::MOUSE_LEFT)) {
                mouse_rotate(delta);
            } 
            if (window->is_mouse_pressed(Button::MOUSE_RIGHT)) {
                mouse_zoom(delta.y);
            }
        }
        float move_speed = 100;
        if (window->is_key_pressed(Button::KEY_W)) {
            m_Position -= forward() * move_speed;
        }
        if (window->is_key_pressed(Button::KEY_S)) {
            m_Position -= forward() * move_speed;
        }
        if (window->is_key_pressed(Button::KEY_A)) {
            m_Position -= right() * move_speed;
        }
        if (window->is_key_pressed(Button::KEY_D)) {
            m_Position += right() * move_speed;
        }
    }

    void Camera::on_event(App* app, Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.bind(this, &Camera::on_mouse_scrolled);
    }

    glm::vec3 Camera::up() const {
        return glm::rotate(orientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 Camera::right() const {
        return glm::rotate(orientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 Camera::forward() const {
        return glm::rotate(orientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::quat Camera::orientation() const {
        return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
    }

    void Camera::update_projection() {
        m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
        m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
        m_Projection[1][1] *= -1;
    }

    void Camera::update_view() {
        m_Position = calculate_position();
        m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation());
        m_ViewMatrix = glm::inverse(m_ViewMatrix);
    }

    bool Camera::on_mouse_scrolled(MouseScrolledEvent& ev) {
        float delta = ev.offset_y() * 0.1f;
        mouse_zoom(delta);
        update_view();
        return false;
    }

    void Camera::mouse_pan(const glm::vec2& delta) {
        glm::vec2 speed = pan_speed();
        m_FocalPoint += -up() * delta.x * speed.x * m_Distance;
        m_FocalPoint += up() * delta.y * speed.y * m_Distance;
    }

    void Camera::mouse_rotate(const glm::vec2& delta) {
        float yaw_sign = up().y < 0 ? -1.0f : 1.0f;
        m_Yaw += yaw_sign * delta.x * rotation_speed();
        m_Pitch += delta.y * rotation_speed();
    }

    void Camera::mouse_zoom(float delta) {
        m_Distance -= delta * zoom_speed();
        if (m_Distance < 1.0f) {
            m_FocalPoint += forward();
            m_Distance = 1.0f;
        }
    }

    glm::vec3 Camera::calculate_position() const {
        return m_FocalPoint - forward() * m_Distance;
    }

    glm::vec2 Camera::pan_speed() const {
        float x = std::min(m_ViewportWidth / 1000.0f, 2.4f);
        float x_factor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
        float y = std::min(m_ViewportHeight / 1000.0f, 2.4f);
        float y_factor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;
        return { x_factor, y_factor };
    }

    float Camera::rotation_speed() const {
        return 0.8f;
    }

    float Camera::zoom_speed() const {
        float distance = m_Distance * 0.2f;
        distance = std::max(distance, 0.0f);
        float speed = distance * distance;
        speed = std::min(speed, 100.0f);
        return speed;
    }

    float Camera::distance() const { return m_Distance; }
    const glm::mat4& Camera::projection() const { return  m_Projection; }
    const glm::mat4& Camera::view_matrix() const { return  m_ViewMatrix; }
    glm::mat4 Camera::view_projection() const { return  m_Projection * m_ViewMatrix; }
    const glm::vec3& Camera::position() const { return  m_Position; }
    float Camera::pitch() const { return  m_Pitch; }
    float Camera::yaw() const { return  m_Yaw; }
    void Camera::set_distance(float distance) { m_Distance = distance; }
    void Camera::set_viewport(float w, float h) { m_ViewportWidth = w;  m_ViewportHeight = h; update_projection(); }

}