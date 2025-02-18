#include "Rendering/UI/Panel.h"
#include "Core/Log.h"

namespace aby::ui {

    Ref<Panel> Panel::create(const Transform& transform, const Style& style) {
        return create_ref<Panel>(transform, style);
    }

    Panel::Panel(const Transform& transform, const Style& style) :
        ParentWidget(0),
        Image(transform, style) {}

    void Panel::on_create(App* app, bool deserialized) {
        m_Transform.size = app->window()->size();
        Image::on_create(app, deserialized);
        bInvalid = Image::on_invalidate();
        ParentWidget::on_create(app, deserialized);
    }

    void Panel::on_tick(App* app, Time deltatime) {
        Image::on_tick(app, deltatime);
        ParentWidget::on_tick(app, deltatime);
    }

    void Panel::on_event(App* app, Event& event) {
        Image::on_event(app, event);
        ParentWidget::on_event(app, event);
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

}
