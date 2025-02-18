#pragma once
#include "Core/Common.h"
#include "Core/Resource.h"
#include <glm/glm.hpp>

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

    enum class EButtonState {
        DEFAULT = 0,
        HOVERED = 1,
        PRESSED = 2,
        RELEASED = DEFAULT,
    };
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

}