#pragma once
#include "Core/Common.h"
#include "Core/Resource.h"
#include <glm/glm.hpp>

namespace aby {
    class App;
}

namespace aby::ui {
    
    enum class EAnchor {
        NONE          = 0,
        TOP_LEFT      = NONE, 
        TOP_MIDDLE    = 1,    
        TOP_RIGHT     = 2,    
        MIDDLE_LEFT   = 3,
        MIDDLE        = 4,
        MIDDLE_RIGHT  = 5,
        BOTTOM_LEFT   = 6,
        BOTTOM_MIDDLE = 7,
        BOTTOM_RIGHT  = 8,
    };
    
    enum class EScaleMode {
        SCALE_TO_FIT = 0,
        FIT_TO_SCALE = 1,
    };

    enum class EDirection {
        HORIZONTAL = 0, // x
        VERTICAL   = 1, // y
    };

    enum class ELayout {
        AUTO = 0, // If horizontal then left to right, else vertical, then top to bottom.
        LEFT_TO_RIGHT,
        TOP_TO_BOTTOM,
        RIGHT_TO_LEFT,
        BOTTOM_TO_TOP
    };

    enum class EButtonState {
        DEFAULT  = 0,
        HOVERED  = 1,
        PRESSED  = 2,
        RELEASED = DEFAULT,
    };

    enum class ETextAlignment {
        CENTER = 0,
        LEFT,
        RIGHT,
    };

    enum class EResize {
        NONE = 0,
        N    = BIT(0),
        E    = BIT(1),
        S    = BIT(2),
        W    = BIT(3),
    };
    DECLARE_ENUM_OPS(EResize);
}

namespace aby::ui {
    struct Anchor {
        EAnchor   position = EAnchor::NONE;
        glm::vec2 offset   = { 0.f, 0.f };
    };
    struct Transform {
        Anchor    anchor   = {};
        glm::vec2 position = { 0.f, 0.f };
        glm::vec2 size     = { 0.f, 0.f };
    };
    
    struct Background {
        glm::vec4 color = { 1.f, 1.f, 1.f, 1.f }; 
        Resource  texture = {};
    };
    struct Border {
        glm::vec4 color = { 0.f, 0.f, 0.f, 1.f };
        float     width = 0.f;
    };
    struct Style {
        Background background = {}; // If opacity is not 1 then border will draw 4 lines instead of a rect.
        Border     border     = {}; 

        static Style dark_mode();  // Default dark mode style.
        static Style light_mode(); // Default light mode style.
    };

    struct ButtonStyle {
        Background hovered  = {}; 
        Background pressed  = {};
        Background released = {}; // released and default are the same.
        Border     border   = {};
        
        static ButtonStyle dark_mode();   // Default dark mode style.
        static ButtonStyle light_mode();  // Default light mode style.
    };

    struct TextInfo {
        std::string    text      = "";
        glm::vec4      color     = { 1, 1, 1, 1 };
        float          scale     = 1.f;
        ETextAlignment alignment = ETextAlignment::CENTER;
    };

    struct InputTextOptions {
        std::size_t prefix       = 0;     // How much of the text is a prefix and therefore should not be deletable.
        bool submit_clears_focus = false; // Does pressing enter/submitting make the input text box lose focus.
        bool submit_clears_text  = true;  // Does pressing enter/submitting clear the text (not including prefix).
    };


    struct ResizeResult {
        float distance;
        EResize direction; // N, E -> Grow. S, W -> Shrink
    };

    class ResizeOperation {
    public:
        ResizeOperation(EResize resizability);
        ~ResizeOperation() = default;

        bool begin(const glm::vec2& mouse_pos);
        void update(App* app, const Transform& transform, const glm::vec2& mouse_pos, float pad = 0.f);
        ResizeResult resize(Transform& transform, const glm::vec2& mouse_pos);
        void reset();

        bool can_resize() const;
        bool is_resizing() const;
        bool should_resize() const;

        void set_resizability(EResize resizability);
    private:
        bool      is      = false;          // Determines if a resize operation is occurring.
        EResize   ability = EResize::NONE;  // Determines the edges allowed to be resized.
        EResize   state   = EResize::NONE;  // Determines which edge is currently being resized
        ECursor   cursor  = ECursor::ARROW; // The cursor to set on a resize operation.
        glm::vec2 start   = { 0, 0 };       // The start position of the mouse.
        glm::vec2 end     = { 0, 0 };       // The end position of the mouse.
    };
}