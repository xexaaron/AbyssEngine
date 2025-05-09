#include "Rendering/UI/UICommon.h"
#include "Core/App.h"

namespace aby::ui {

    ImageStyle ImageStyle::dark_mode() {
        return ImageStyle{
            .border  = { { 0.1f, 0.1f, 0.1f, 0.5f}, 2.0f },
            .color   = { 0.15f, 0.15f, 0.15f, 1.f },
            .texture = {}
        };
    }

    ImageStyle ImageStyle::light_mode() {
        return ImageStyle{
            .border  = { { 0.6f, 0.6f, 0.6f, 1.f }, 2.0f },
            .color   = { 0.90f, 0.90f, 0.90f, 1.f },
            .texture = {}
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
            .hovered  = { { 0.85f, 0.85f, 0.85f, 1.f }, {} }, // Light gray when hovered
            .pressed  = { { 0.70f, 0.70f, 0.70f, 1.f }, {} }, // Darker gray when pressed
            .released = { { 0.90f, 0.90f, 0.90f, 1.f }, {} }, // Default light gray
            .border   = { { 0.6f, 0.6f, 0.6f, 1.f }, 2.0f }  // Medium gray border
        };
    }

    ResizeOperation::ResizeOperation(EResize resizability) {
        this->is      = false;
        this->ability = resizability;
        this->state   = EResize::NONE;
        this->cursor  = ECursor::ARROW;
        this->start   = { 0, 0 };
        this->end     = { 0, 0 };
    }


    bool ResizeOperation::begin(const glm::vec2& mouse_pos) {
        if (this->is) return false;
        this->is = true;
        this->start = mouse_pos;
        return true;
    }

    void ResizeOperation::reset() {
        this->is     = false;
        this->state  = EResize::NONE;
        this->cursor = ECursor::ARROW;
    }

    void ResizeOperation::update(App* app, const Transform& transform, const glm::vec2& mouse_pos, float pad) {
        const float left = transform.position.x;
        const float right = left + transform.size.x;
        const float top = transform.position.y;
        const float bottom = top + transform.size.y;
        const float border = pad / 2.f;
        do
        {
            if ((this->ability & EResize::N) != EResize::NONE) {
                if (mouse_pos.x >= left         && mouse_pos.x <= right &&
                    mouse_pos.y >= top - border && mouse_pos.y <= top + border)
                {
                    this->state = EResize::N;
                    this->cursor = ECursor::VRESIZE;
                    break;
                }
            }

            if ((this->ability & EResize::S) != EResize::NONE) {
                if (mouse_pos.x >= left            && mouse_pos.x <= right &&
                    mouse_pos.y >= bottom - border && mouse_pos.y <= bottom + border)
                {
                    this->state = EResize::S;
                    this->cursor = ECursor::VRESIZE;
                    break;
                }
            }

            if ((this->ability & EResize::W) != EResize::NONE) {
                if (mouse_pos.x >= left - border && mouse_pos.x <= left + border &&
                    mouse_pos.y >= top           && mouse_pos.y <= bottom)
                {
                    this->state = EResize::W;
                    this->cursor = ECursor::HRESIZE;
                    break;
                }
            }

            if ((this->ability & EResize::E) != EResize::NONE) {
                if (mouse_pos.x >= right - border && mouse_pos.x <= right + border &&
                    mouse_pos.y >= top            && mouse_pos.y <= bottom)
                {
                    this->state = EResize::E;
                    this->cursor = ECursor::HRESIZE;
                    break;
                }
            }

            this->state  = EResize::NONE;
            this->cursor = ECursor::ARROW;
        } while (0);

        app->window()->set_cursor(this->cursor);
    }

    ResizeResult ResizeOperation::resize(Transform& transform, const glm::vec2& mouse_pos) {
        const auto distance = this->end - this->start;
        EResize direction = EResize::NONE;
        float amt = 0.f;

        switch (this->state) {
            case EResize::N:
                transform.size.y -= distance.y;
                transform.position.y += distance.y;
                direction = distance.y < 0.f ? EResize::N : EResize::S;
                amt = distance.y;
                this->start = this->end;
                break;
            case EResize::E:
                transform.size.x += distance.x;
                direction = distance.x < 0.f ? EResize::E : EResize::W;
                amt = distance.x;
                this->start = this->end;
                break;
            case EResize::S:
                transform.size.y += distance.y;
                direction = distance.y < 0.f ? EResize::N : EResize::S;
                amt = distance.y;
                this->start = this->end;
                break;
            case EResize::W:
                transform.size.x -= distance.x;
                transform.position.x += distance.x;
                direction = distance.x < 0.f ? EResize::E : EResize::W;
                amt = distance.x;
                this->start = this->end;
                break;
            default:
                return ResizeResult{
                    .distance = amt,
                    .direction = direction
                };
        }

        this->end = mouse_pos;

        return ResizeResult {
            .distance  = amt,
            .direction = direction,
        };
    }

    bool ResizeOperation::can_resize() const {
        return this->ability != EResize::NONE;
    }

    bool ResizeOperation::is_resizing() const {
        return this->is;
    }

    bool ResizeOperation::should_resize() const {
        return this->state != EResize::NONE && this->is;
    }

    void ResizeOperation::set_resizability(EResize resizability) {
        ABY_ASSERT(!this->is, "Cannot set resizability during a resize operation!");
        this->ability = resizability;
    }



}
namespace std {
    string to_string(aby::ui::EAnchor a) {
        switch (a) {
            using aby::ui::EAnchor;
            case EAnchor::NONE:
                return "N/A";
            case EAnchor::TOP_LEFT:
                return "Top Left";
            case EAnchor::TOP_CENTER:
                return "Top Center";
            case EAnchor::TOP_RIGHT:
                return "Top Right";
            case EAnchor::CENTER_LEFT:
                return "Center Left";
            case EAnchor::CENTER:
                return "Center";
            case EAnchor::CENTER_RIGHT:
                return "Center Right";
            case EAnchor::BOTTOM_LEFT:
                return "Bottom Left";
            case EAnchor::BOTTOM_CENTER:
                return "Bottom Center";
            case EAnchor::BOTTOM_RIGHT:
                return "Bottom Right";
            default:
                throw std::out_of_range("aby::ui::EAnchor");
        }
    }
}