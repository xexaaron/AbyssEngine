#include "Rendering/UI/Textbox.h"
#include "Core/Log.h"

namespace aby::ui {
    Ref<Textbox> Textbox::create(const Transform& transform, const Style& style, const TextInfo& info) {
        return create_ref<Textbox>(transform, style, info);
    }

    Textbox::Textbox(const Transform& transform, const Style& style, const TextInfo& info) :
        Image(transform, style),
        m_Text(info.text, transform.position + (transform.size / glm::vec2(2.f, 2.f)), info.color, info.scale, 0),
        m_Alignment(info.alignment),
        m_TextSize(0.f, 0.f)
    {

    }

    void Textbox::on_create(App* app, bool deserialized) {
        Image::on_create(app, deserialized);
        m_Font = app->ctx().fonts().at(Resource{ EResource::FONT, m_Text.font });
        if (auto font = m_Font.lock()) {
            m_TextSize = font->measure(m_Text.text) * m_Text.scale;
        }
        invalidate_self();
    }

    void Textbox::on_tick(App* app, Time deltatime) {
        if (!bVisible) return;
        Image::on_tick(app, deltatime);
        app->renderer().draw_text(m_Text);
    }
            
    bool Textbox::on_invalidate() {
        auto result = Image::on_invalidate(); 
        glm::vec3 pos    = glm::vec3(m_Transform.position + (m_Transform.size / 2.f), 0.f);
        glm::vec3 offset = glm::vec3(m_TextSize / 2.f, 0.f);

        switch (m_Alignment) {
            using enum ETextAlignment;
            case CENTER:
            {
                m_Text.pos = pos - offset;
                break;
            }
            case LEFT:
            {
                m_Text.pos.x = m_Transform.position.x;
                m_Text.pos.y = pos.y - offset.y; // No offset in the x direction
                break;
            }
            case RIGHT:
            {
                m_Text.pos.x = m_Transform.position.x + m_Transform.size.x - m_TextSize.x;
                m_Text.pos.y = pos.y - offset.y;
                break;
            }
        }

        return result;
    }
    
    void Textbox::set_text(const std::string& text, Ref<Font> font) {
        if (auto my_font = m_Font.lock(); my_font && font) {
            if (font != my_font) {
                m_Font = font;
            }
        }
        m_Text.text = text;
        m_TextSize  = m_Font.lock()->measure(text);
    }

    bool Textbox::on_window_resize(WindowResizeEvent& event) {
        invalidate_self();
        return false;
    }

    glm::vec2 Textbox::text_size() {
        return m_TextSize;
    }


}
