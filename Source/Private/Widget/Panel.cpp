#include "Widget/Panel.h"

namespace aby::ui {

    Ref<Panel> Panel::create(const Transform& transform, const ImageStyle& style, EResize resizability) {
        return create_ref<Panel>(transform, style, resizability);
    }

    Panel::Panel(const Transform& transform, const ImageStyle& style, EResize resizability) :
        Image(transform, style),
        m_Resize(resizability),
        m_App(nullptr)
    {
        m_Name = "Panel";
    }

    void Panel::on_create(App* app, bool deserialized) {
        Super::on_create(app, deserialized);
        m_App = app;
    }

    void Panel::on_event(App* app, Event& event) {
        Super::on_event(app, event);
        if (!bVisible) return;
        EventDispatcher dsp(event);
        if (m_Resize.can_resize())
        {
            dsp.bind(this, &Panel::on_mouse_pressed);
            dsp.bind(this, &Panel::on_mouse_moved);
            dsp.bind(this, &Panel::on_mouse_released);
        }
    }

    bool Panel::on_mouse_pressed(MousePressedEvent& event) {
        m_Resize.begin(event.pos());
        return true;
    }

    bool Panel::on_mouse_released(MouseReleasedEvent& event) {
        m_Resize.reset();
        return false;
    }

    bool Panel::on_mouse_moved(MouseMovedEvent& event) {
        const auto loc = event.pos();
        m_Resize.update(m_App, m_Transform, loc, m_Border.width);
        if (m_Resize.should_resize())  {
            ResizeResult result = m_Resize.resize(m_Transform, loc);
            on_resize(result.direction, result.distance);
        }
        return false;
    }

    void Panel::set_resizability(EResize resizability) {
        m_Resize.set_resizability(resizability);
    }

    void Panel::on_resize(EResize direction, float distance) {

    }

}