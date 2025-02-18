#include "Rendering/UI/UICommon.h"

namespace aby::ui {

    Style Style::dark_mode() {
        return Style{
            .background = Background{
                .color = {0.15f, 0.15f, 0.15f, 1.f },
                .texture = {}
            },
            .border = {
                .color = { 0.1f, 0.1f, 0.1f, 1.f },
                .width = 2.f
            }
        };
    }

    Style Style::light_mode() {
        return Style{
            .background = Background{
                .color = {1.0f, 1.0f, 1.0f, 1.f }, 
                .texture = {}
            },
            .border = {
                .color = {0.8f, 0.8f, 0.8f, 1.f }, 
                .width = 2.f
            }
        };
    }


    ButtonStyle ButtonStyle::dark_mode() {
        return ButtonStyle{
            .hovered  = { { 0.25f, 0.25f, 0.25f, 1.f }, {} }, // Slightly lighter gray
            .pressed  = { { 0.10f, 0.10f, 0.10f, 1.f }, {} }, // Even darker gray for a "pressed" effect
            .released = { { 0.15f, 0.15f, 0.15f, 1.f }, {} }, // Dark gray background
            .border   = { { 0.8f, 0.8f, 0.8f, 0.5f }, 2.0f }  // Light gray border for contrast
        };
    }
    ButtonStyle ButtonStyle::light_mode() {
        return ButtonStyle{
            .hovered = { { 0.85f, 0.85f, 0.85f, 1.f }, {} }, // Light gray when hovered
            .pressed = { { 0.70f, 0.70f, 0.70f, 1.f }, {} }, // Darker gray when pressed
            .released = { { 0.90f, 0.90f, 0.90f, 1.f }, {} }, // Default light gray
            .border = { { 0.6f, 0.6f, 0.6f, 1.f }, 2.0f }  // Medium gray border
        };
    }
}