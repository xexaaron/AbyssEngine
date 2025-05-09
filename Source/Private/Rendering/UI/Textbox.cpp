#include "Rendering/UI/Textbox.h"

namespace aby::ui {
    Ref<Textbox> Textbox::create(const Transform& transform, const ImageStyle& style, const TextInfo& info) {
        return create_ref<Textbox>(transform, style, info);
    }

    Textbox::Textbox(const Transform& transform, const ImageStyle& style, const TextInfo& info) :
        Image(transform, style),
        m_Text(info.text, transform.position + (transform.size / glm::vec2(2.f, 2.f)), info.color, info.scale, 0),
        m_Alignment(info.alignment),
        m_TextSize(0.f, 0.f)
    {
        m_Name = "Textbox";
        bScalesWithWindow = false;
    }

    void Textbox::on_create(App* app, bool deserialized) {
        Image::on_create(app, deserialized);
        m_Font = app->ctx().fonts().at(Resource{ EResource::FONT, m_Text.font });
        if (auto font = m_Font.lock()) {
            m_TextSize = font->measure(m_Text.prefix + m_Text.text) * m_Text.scale;
        }
    }

    void Textbox::on_tick(App* app, Time deltatime) {
        if (!bVisible) return;

        glm::vec3 pos    = glm::vec3(m_Transform.position + (m_Transform.size / 2.f), 0.f);
        glm::vec3 offset = glm::vec3(m_TextSize / 2.f, 0.f);

        switch (m_Alignment) {
            case ETextAlignment::CENTER: {
                m_Text.pos = pos - offset;
                break;
            }
            case ETextAlignment::LEFT: {
                m_Text.pos.x = m_Transform.position.x;
                m_Text.pos.y = pos.y - offset.y; // No offset in the x direction
                break;
            }
            case ETextAlignment::RIGHT: {
                m_Text.pos.x = m_Transform.position.x + m_Transform.size.x - m_TextSize.x;
                m_Text.pos.y = pos.y - offset.y;
                break;
            }
        }

        Image::on_tick(app, deltatime);
        app->renderer().draw_text(m_Text);
    }
            
    std::size_t Textbox::hit_text(const glm::vec2& mouse_pos) {
        const auto prefix_len = m_Text.prefix.length();
        const auto total_len = prefix_len + m_Text.text.length();

        if (total_len == 0)
            return std::string::npos;

        auto font = m_Font.lock();
        if (!font) {
            ABY_ERR("Font object inaccessible (id: {})", m_Text.font);
            return std::string::npos;
        }

        if (!font->is_mono()) {
            throw std::runtime_error("TODO: Handle non monospaced hit transforms for text character selection");
        }

        bool hit = (mouse_pos.x >= m_Text.pos.x && mouse_pos.x <= m_Text.pos.x + m_TextSize.x) &&
            (mouse_pos.y >= m_Text.pos.y && mouse_pos.y <= m_Text.pos.y + m_TextSize.y);

        if (!hit)
            return std::string::npos;

        float relative_x = mouse_pos.x - m_Text.pos.x;
        std::size_t char_index = static_cast<std::size_t>(relative_x / font->char_width());

        if (char_index < prefix_len) {
            return 0;
        }
        else {
            return std::min(char_index - prefix_len, m_Text.text.size());
        }
    }

    const Text& Textbox::text() const {
        return m_Text;
    }


    glm::vec2 Textbox::text_size() {
        return m_TextSize;
    }

    void Textbox::set_text(const std::string& text, Ref<Font> font) {
        if (auto my_font = m_Font.lock(); my_font && font) {
            if (font != my_font) {
                m_Font = font;
            }
        }

        m_Text.text = text;
        if (auto font_obj = m_Font.lock()) {
            m_TextSize = font_obj->measure(text);
        }
    }


}