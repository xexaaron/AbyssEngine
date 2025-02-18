#include "Rendering/UI/Textbox.h"
#include "Core/Log.h"

namespace aby::ui {
    Ref<Textbox> Textbox::create(const Transform& transform, const Style& style, const std::string& text, float scale) {
        return create_ref<Textbox>(transform, style, text, scale);
    }

    Textbox::Textbox(const Transform& transform, const Style& style, const std::string& text, float scale) :
        Image(transform, style),
        m_Text(text, transform.position + (transform.size / glm::vec2(2, 2)), scale, 0),
        m_TextSize(0, 0)
    {
    }

    void Textbox::on_create(App* app, bool deserialized) {
        Image::on_create(app, deserialized);
        m_Font = app->ctx().fonts().at(Resource{ EResource::FONT, m_Text.font });
        on_invalidate();
    }

    void Textbox::on_tick(App* app, Time deltatime) {
        Image::on_tick(app, deltatime);
        app->renderer().draw_text(m_Text);
    }
            
    bool Textbox::on_invalidate() {
        auto result = Image::on_invalidate();

        m_TextSize = m_Font->measure(m_Text.text) * m_Text.scale;
        // if (m_TextSize.x > m_Transform.size.x || m_TextSize.y > m_Transform.size.y) {
        //     float scaleX = m_Transform.size.x / m_TextSize.x;
        //     float scaleY = m_Transform.size.y / m_TextSize.y;
        //     m_Text.scale = glm::min(scaleX, scaleY);  // Shrink to fit
        //     m_TextSize = m_Font->measure(m_Text.text) * m_Text.scale;
        // }
        glm::vec3 offset = glm::vec3(m_TextSize / 2.f, 0.f);
        m_Text.pos = glm::vec3(m_Transform.position + (m_Transform.size / 2.f), 0.f) - offset;

        return result;
    }
    void Textbox::set_text(const std::string& text, Ref<Font> font) {
        m_Text.text = text;
        m_TextSize  = font->measure(text);
    }

}