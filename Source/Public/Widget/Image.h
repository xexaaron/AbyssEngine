#pragma once
#include "Widget/Widget.h"
#include <array>

namespace aby::ui {
    
    class Image : public Widget {
    public:
        static Ref<Image> create(const Transform& transform, const ImageStyle& style);
        Image(const Transform& transform, const ImageStyle& style);

        void on_tick(App* app, Time deltatime) override;
    
        const Border& border() const;

        void set_texture(Resource texture);
        void set_border(const Border& border);
        void set_background(const glm::vec4& background);
        void set_style(const ImageStyle& style);
    protected:
        bool      bUnifiedBorder;
        Resource  m_Texture;
        glm::vec4 m_Background;
        Border    m_Border;
    };
}