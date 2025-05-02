#include "Rendering/Camera.h"
#include "Core/App.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace aby {

    ICamera::ICamera(const Config& cfg, const glm::vec3& pos) :
        m_Pitch(0.f),
        m_Yaw(0.f),
        m_Roll(0.f),
        m_AspectRatio(cfg.AspectRatio),
        m_FOV(cfg.FOV),
        m_NearClip(cfg.NearClip),
        m_FarClip(cfg.FarClip),
        m_RotationSpeed(0.8f),
        m_ZoomSpeed(1.0f),
        m_InitialMousePosition(glm::vec2(0.f)),
        m_ViewportSize(glm::vec2(0.f)),
        m_Position(pos),
        m_ViewMatrix(glm::mat4(1.0f)),
        m_Projection(glm::perspective(glm::radians(cfg.FOV), cfg.AspectRatio, cfg.AspectRatio, cfg.FarClip))
    {
    }

    void ICamera::on_tick(App* app, Time  deltatime) {
        update_view();
    }

    void ICamera::on_event(App* app, Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.bind(this, &ICamera::on_mouse_scrolled);
    }
    bool ICamera::on_mouse_scrolled(MouseScrolledEvent& ev) {
        float delta = ev.offset_y() * 0.1f;
        on_zoom(delta);
        update_view();
        return false;
    }

    void ICamera::update_projection() {
        m_AspectRatio = m_ViewportSize.x / m_ViewportSize.y;
        m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
        m_Projection[1][1] *= -1;
    }

    void ICamera::set_viewport(const glm::vec2& viewport_size) { m_ViewportSize = viewport_size; }
    void ICamera::set_rotation_speed(float speed) { m_RotationSpeed = speed; }
    void ICamera::set_zoom_speed(float speed) { m_ZoomSpeed = speed; }
    float ICamera::rotation_speed() const { return m_RotationSpeed; }
    float ICamera::zoom_speed() const { return m_ZoomSpeed; }
    float ICamera::near_clip() const { return m_NearClip; }
    float ICamera::far_clip() const { return m_FarClip; }
    float ICamera::fov() const { return m_FOV; }
    float ICamera::aspect_ratio() const { return m_AspectRatio; }
    float ICamera::pitch() const { return  m_Pitch; }
    float ICamera::yaw() const { return  m_Yaw; }
    const glm::vec2& ICamera::viewport_size() const { return m_ViewportSize; }
    glm::vec3 ICamera::up() const { return glm::rotate(orientation(), glm::vec3(0.0f, 1.0f, 0.0f)); }
    glm::vec3 ICamera::right() const { return glm::rotate(orientation(), glm::vec3(1.0f, 0.0f, 0.0f)); }
    glm::vec3 ICamera::forward() const { return glm::rotate(orientation(), glm::vec3(0.0f, 0.0f, 1.0f)); }
    const glm::vec3& ICamera::position() const { return  m_Position; }
    glm::quat ICamera::orientation() const { return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, -m_Roll)); }
    glm::mat4 ICamera::view_projection() const { return  m_Projection * m_ViewMatrix; }
    
    void ICamera::debug() const {
        ABY_DBG("Camera:");
        ABY_DBG(" -- Position: {}", m_Position);
        ABY_DBG(" -- Rotation: (yaw:{}, pitch:{}, roll:{})", m_Yaw, m_Pitch, 0.f);
        ABY_DBG(" -- Viewport: {}", m_ViewportSize);
    }
    void ICamera::look_at(const glm::vec3& target) {
        glm::vec3 direction = glm::normalize(target - m_Position);
        m_Yaw = glm::degrees(std::atan2(direction.x, direction.z));
        m_Pitch = glm::degrees(std::asin(direction.y));
        update_view();
    }

}

namespace aby {

    OrientedCamera::OrientedCamera(const Config& cfg, const glm::vec3& focal_point, float distance_from_focal_point) :
        ICamera(cfg),
        m_Distance(distance_from_focal_point),
        m_FocalPoint(focal_point)
    {
        update_view();
    }

    void OrientedCamera::on_tick(App* app, Time deltatime) {
        ICamera::on_tick(app, deltatime);
        auto window = app->window();
        if (window->is_key_pressed(Button::KEY_LEFT_ALT)) {
            auto mouse_pos = window->mouse_pos();
            glm::vec2 delta = (mouse_pos - m_InitialMousePosition) * 0.003f;
            m_InitialMousePosition = mouse_pos;
            if (window->is_mouse_pressed(Button::MOUSE_MIDDLE)) {
                on_pan(delta);
            }
            if (window->is_mouse_pressed(Button::MOUSE_LEFT)) {
                on_rotate(delta);
            }
            if (window->is_mouse_pressed(Button::MOUSE_RIGHT)) {
                on_zoom(delta.y);
            }
        }
    }
    void OrientedCamera::on_rotate(const glm::vec2& delta) {
        float yaw_sign = up().y < 0 ? -1.0f : 1.0f;
        m_Yaw += yaw_sign * delta.x * m_RotationSpeed;
        m_Pitch += delta.y * m_RotationSpeed;
    }

    void OrientedCamera::on_pan(const glm::vec2& delta) {
        float x = std::min(m_ViewportSize.x / 1000.0f, 2.4f);
        float x_speed = 0.0366f * (x * x) - (m_AspectRatio * 0.1f) * x + 0.3021f;
        float y = std::min(m_ViewportSize.y / 1000.0f, 2.4f);
        float y_speed = 0.0366f * (y * y) - (m_AspectRatio * 0.1f) * y + 0.3021f;
        m_FocalPoint += -up() * delta.x * x_speed * m_Distance;
        m_FocalPoint += up() * delta.y * y_speed * m_Distance;
    }

    void OrientedCamera::on_zoom(float delta) {
        float distance = m_Distance * 0.2f;
        distance = std::max(distance, 0.0f);
        float speed = distance * distance;
        speed = std::min(speed, 100.0f);
        m_Distance -= delta * speed * m_ZoomSpeed;
        if (m_Distance < 1.0f) {
            m_FocalPoint += forward();
            m_Distance = 1.0f;
        }
    }
    void OrientedCamera::set_distance(float distance) {
        m_Distance = distance;
    }

    void OrientedCamera::set_focal_point(const glm::vec3& focal_point) {
        m_FocalPoint = focal_point;
    }
    float OrientedCamera::distance() const {
        return m_Distance;
    }

    const glm::vec3& OrientedCamera::focal_point() const {
        return m_FocalPoint;
    }

    ECamera OrientedCamera::type() const {
        return ECamera::ORIENTED;
    }

    void OrientedCamera::update_view() {
        m_Position = m_FocalPoint - forward() * m_Distance;
        m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation());
        m_ViewMatrix = glm::inverse(m_ViewMatrix);
        m_ViewMatrix[1][1] *= -1;
    }

    
}

namespace aby {
    FreeCamera::FreeCamera(const Config& cfg) :
        ICamera(cfg, glm::vec3(200.f, 200.f, 200.f))
    {
        look_at({ 0.f, 0.f, 0.f });
        update_view();
    }

    void FreeCamera::on_zoom(float delta) {
        m_Position += forward() * delta * 100.f * m_ZoomSpeed;
    }

    void FreeCamera::on_rotate(const glm::vec2& delta) {
        float yaw_sign = up().y < 0 ? -1.0f : 1.0f;
        m_Yaw += yaw_sign * delta.x * m_RotationSpeed;
        m_Pitch += delta.y * m_RotationSpeed;
        m_Pitch = glm::clamp(m_Pitch, -89.0f, 89.0f);
    }

    void FreeCamera::on_tick(App* app, Time time) {
        ICamera::on_tick(app, time);
        auto window = app->window();
        float move_speed = 100;
        if (window->is_key_pressed(Button::KEY_LEFT_ALT)) {
            auto mouse_pos = window->mouse_pos();
            glm::vec2 delta = (mouse_pos - m_InitialMousePosition) * 0.003f;
            m_InitialMousePosition = mouse_pos;
            if (window->is_mouse_pressed(Button::MOUSE_LEFT)) {
                on_rotate(delta);
            }
            if (window->is_mouse_pressed(Button::MOUSE_RIGHT)) {
                on_zoom(delta.y);
            }
        }
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
        if (window->is_key_pressed(Button::KEY_Q)) {
            m_Roll += rotation_speed() * time; // or any time delta
        }
        if (window->is_key_pressed(Button::KEY_E)) {
            m_Roll -= rotation_speed() * time;
        }
    }

    ECamera FreeCamera::type() const {
        return ECamera::FREE;
    }

    void FreeCamera::update_view() {
        m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation());
        m_ViewMatrix = glm::inverse(m_ViewMatrix);
        m_ViewMatrix[1][1] *= -1;
    }

}