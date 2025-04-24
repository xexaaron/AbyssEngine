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
        bool on_window_resize(WindowResizeEvent& event) override;

        /**
        * @brief Check if mouse position hit text transform.
        * @return Success: Character index
        * @return Failure: std::string::npos
        */
        std::size_t hit_text(const glm::vec2& mouse_pos);

        const Text& text() const;
        glm::vec2 text_size();

        void set_text(const std::string& text, Ref<Font> font = nullptr);
    protected:
        Text m_Text;
        ETextAlignment m_Alignment;
        glm::vec2 m_TextSize;
        Weak<Font> m_Font;
    };

}