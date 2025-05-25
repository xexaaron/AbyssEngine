#include "Widget/Image.h"
#include "Core/Log.h"

namespace aby::ui {
    Ref<Image> Image::create(const Transform& transform, const ImageStyle& style) {
        return create_ref<Image>(transform, style);
    }

    Image::Image(const Transform& transform, const ImageStyle& style) :
        Widget(transform),
        bUnifiedBorder((style.color.a != 1.f)),
        m_Texture(style.texture),
        m_Background(style.color),
        m_Border(style.border)
    {
        m_Name = "Image";
    }

    void Image::on_tick(App* app, Time deltatime) {
        if (!bVisible) return;

        if (bUnifiedBorder) {
            Quad border(
                m_Transform.size,
                m_Transform.position,
                m_Border.color,
                0.f
            );
            app->renderer().draw_quad(border);
        }
        else {
            Quad top(
                glm::vec2(m_Transform.size.x, m_Border.width),
                m_Transform.position,
                m_Border.color,
                0.f
            );
            Quad bottom(
                glm::vec2(m_Transform.size.x, m_Border.width),
                m_Transform.position + glm::vec2(0.f, m_Transform.size.y - m_Border.width),
                m_Border.color,
                0.f
            );
            Quad left(
                glm::vec2(m_Border.width, m_Transform.size.y),
                m_Transform.position,
                m_Border.color,
                0.f
            );
            Quad right(
                glm::vec2(m_Border.width, m_Transform.size.y),
                m_Transform.position + glm::vec2(m_Transform.size.x - m_Border.width, 0.f),
                m_Border.color,
                0.f
            );
            app->renderer().draw_quad(top);
            app->renderer().draw_quad(bottom);
            app->renderer().draw_quad(left);
            app->renderer().draw_quad(right);

        }

        Quad img(
            m_Transform.size - (m_Border.width * 2.f),
            m_Transform.position + m_Border.width,
            m_Background,
            static_cast<float>(m_Texture.handle()),
            {1, 1}
        );
        app->renderer().draw_quad(img);

        Super::on_tick(app, deltatime);
    }

    void Image::set_border(const Border& border) {
        m_Border = border;
    }

    void Image::set_background(const glm::vec4& background) {
        m_Background   = background;
        bUnifiedBorder = background.a != 1.f;
    }

    void Image::set_style(const ImageStyle& style) {
        m_Texture      = style.texture;
        m_Background   = style.color;
        bUnifiedBorder = style.color.a != 1.f;
        m_Border       = style.border;
    }

    void Image::set_texture(Resource texture) {
        m_Texture = texture;
    }

    const Border& Image::border() const {
        return m_Border;
    }



}