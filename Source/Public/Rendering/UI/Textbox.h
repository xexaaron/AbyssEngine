#pragma once
#include "Rendering/UI/Image.h"

namespace aby::ui {
    
    class Textbox : public Image {
    public:
        static Ref<Textbox> create(const Transform& transform, const Style& style, const TextInfo& info);

        Textbox(const Transform& transform, const Style& style, const TextInfo& info);
        
        void on_create(App* app, bool deserialized) override;
        void on_tick(App* app, Time deltatime) override;
        bool on_invalidate() override;


        void set_text(const std::string& text, Ref<Font> font = nullptr);
        bool on_window_resize(WindowResizeEvent& event) override;
        glm::vec2 text_size();
    protected:
        Text m_Text;
        ETextAlignment m_Alignment;
        glm::vec2 m_TextSize;
        Weak<Font> m_Font;
    };

}