#pragma once
#include "Rendering/UI/Image.h"

namespace aby::ui {

    class Textbox : public Image {
    public:
        static Ref<Textbox> create(const Transform& transform, const Style& style, const std::string& text, float scale = 1.f);

        Textbox(const Transform& transform, const Style& style, const std::string& text, float scale = 1.f);

        void on_create(App* app, bool deserialized) override;
        void on_tick(App* app, Time deltatime) override;
        bool on_invalidate() override;

        void set_text(const std::string& text, Ref<Font> font);
    protected:
        Text m_Text;
        glm::vec2 m_TextSize;
        Ref<Font> m_Font;
    };

}