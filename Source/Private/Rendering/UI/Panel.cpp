#include "Rendering/UI/Panel.h"

namespace aby::ui {

    Ref<Panel> Panel::create(const Transform& transform, const Style& style, EResize resizability) {
        return create_ref<Panel>(transform, style, resizability);
    }

    Panel::Panel(const Transform& transform, const Style& style, EResize resizability) :
        Image(transform, style),
        ParentWidget(0),
        m_Resize(resizability),
        m_App(nullptr)
    {
    }

    void Panel::on_create(App* app, bool deserialized) {
        Image::on_create(app, deserialized);
        ParentWidget::on_create(app, deserialized);
        m_App = app;
        invalidate_self();
    }


    void Panel::on_event(App* app, Event& event) {
        Image::on_event(app, event);
        if (!bVisible) return;
        ParentWidget::on_event(app, event);
        EventDispatcher dsp(event);
        if (m_Resize.can_resize())
        {
            dsp.bind(this, &Panel::on_mouse_pressed);
            dsp.bind(this, &Panel::on_mouse_moved);
            dsp.bind(this, &Panel::on_mouse_released);
        }
    }

    void Panel::on_destroy(App* app) {
        Image::on_destroy(app);
        ParentWidget::on_destroy(app);
    }

    bool Panel::on_invalidate() {
        if (bInvalid) {
            bInvalid = Image::on_invalidate();
        }
        ParentWidget::on_invalidate();
        return bInvalid;
    }

    void Panel::on_tick(App* app, Time deltatime) {
        if (!bVisible) return;
        Image::on_tick(app, deltatime);
        ParentWidget::on_tick(app, deltatime);
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
        m_Resize.update(m_App, m_Transform, loc, m_Style.border.width);
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
