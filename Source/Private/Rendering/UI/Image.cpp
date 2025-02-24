#include "Rendering/UI/Image.h"
#include "Core/Log.h"

namespace aby::ui {
    Ref<Image> Image::create(const Transform& transform, const Style& style, Ref<Widget> parent) {
        return create_ref<Image>(transform, style, parent);
    }

    Image::Image(const Transform& transform, const Style& style, Ref<Widget> parent) :
        Widget(transform, style, parent),
        m_BorderCt(0) 
    {
    }

    void Image::on_tick(App* app, Time deltatime) {
        if (!bVisible) return;
        for (std::size_t i = 0; i < m_BorderCt; i++) {
            app->renderer().draw_quad_2d(m_Border[i]);
        }
        app->renderer().draw_quad_2d(m_Image);
    }

    bool Image::on_invalidate() {
        if (m_Transform.anchor.position != EAnchor::NONE) {
            m_Transform.position = m_Transform.anchor.offset;
            auto parent = m_Parent.lock();
            ABY_ASSERT(parent, "aby::ui::Image has no parent to be anchor to");
            auto parent_widget = parent->as<Widget>();
            auto parent_size = parent_widget->transform().size;
            switch (m_Transform.anchor.position) {
                case EAnchor::TOP_MIDDLE:
                    m_Transform.position.x += (parent_size.x / 2) - (m_Transform.size.x / 2);
                    break;
                case EAnchor::TOP_RIGHT:
                    m_Transform.position.x += (parent_size.x) - (m_Transform.size.x);
                    break;
                case EAnchor::MIDDLE_LEFT:
                    m_Transform.position.y += (parent_size.y / 2) - (m_Transform.size.y / 2);
                    break;
                case EAnchor::MIDDLE:
                    m_Transform.position.x += (parent_size.x / 2) - (m_Transform.size.x / 2);
                    m_Transform.position.y += (parent_size.y / 2) - (m_Transform.size.y / 2);
                    break;
                case EAnchor::MIDDLE_RIGHT:
                    m_Transform.position.x += (parent_size.x) - (m_Transform.size.x);
                    m_Transform.position.y += (parent_size.y / 2) - (m_Transform.size.y / 2);
                    break;
                case EAnchor::BOTTOM_LEFT:
                    m_Transform.position.y += (parent_size.y) - (m_Transform.size.y);
                    break;
                case EAnchor::BOTTOM_MIDDLE:
                    m_Transform.position.x += (parent_size.x / 2) - (m_Transform.size.x /2);
                    m_Transform.position.y += (parent_size.y) - (m_Transform.size.y);
                    break;
                case EAnchor::BOTTOM_RIGHT:
                    m_Transform.position.x += (parent_size.x) - (m_Transform.size.x);
                    m_Transform.position.y += (parent_size.y) - (m_Transform.size.y);
                    break;
            }
        }

        m_Image = Quad(
            m_Transform.size - glm::vec2(m_Style.border.width * 2.f),
            m_Transform.position + glm::vec2(m_Style.border.width),
            m_Style.background.color,
            static_cast<float>(m_Style.background.texture.handle())
        );
        
        if (m_Style.border.width > 0.f) {
            if (m_Style.background.color.a <= 0.f) {
                m_Border[0] = Quad(
                    glm::vec2(m_Transform.size.x, m_Style.border.width),
                    m_Transform.position,
                    m_Style.border.color,
                    0.f
                );
                // Bottom border
                m_Border[1] = Quad(
                    glm::vec2(m_Transform.size.x, m_Style.border.width),
                    m_Transform.position + glm::vec2(0.f, m_Transform.size.y - m_Style.border.width),
                    m_Style.border.color,
                    0.f
                );
                // Left border
                m_Border[2] = Quad(
                    glm::vec2(m_Style.border.width, m_Transform.size.y),
                    m_Transform.position,
                    m_Style.border.color,
                    0.f
                );
                // Right border
                m_Border[3] = Quad(
                    glm::vec2(m_Style.border.width, m_Transform.size.y),
                    m_Transform.position + glm::vec2(m_Transform.size.x - m_Style.border.width, 0.f),
                    m_Style.border.color,
                    0.f
                );
                m_BorderCt = 4;
            }
            else {
                m_Border[0] = Quad(
                    m_Transform.size,
                    m_Transform.position,
                    m_Style.border.color,
                    0.f
                );
                m_BorderCt = 1;

            }
        }
       
        return false;
    }
}