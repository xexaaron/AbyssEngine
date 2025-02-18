#include <iostream>
#include <filesystem>
#include "Core/App.h"
#include "Core/Log.h"
#include "Core/Serialize.h"
#include "Rendering/Viewport.h"
#include "Core/Resource.h"
#include "Rendering/UI/Widget.h"
#include "Rendering/Font.h"
#include <fstream>

constexpr std::uint32_t operator""pt(unsigned long long x) {
    return static_cast<std::uint32_t>(x);
}

int main(int argc, char* argv[]) {
    aby::App app(aby::AppInfo{ 
        .Name     = "Abyss",
        .Version  = aby::AppVersion{ 
            .Major = 0, 
            .Minor = 1,
            .Patch = 0
        },
        .bInherit = true,
        .Backend  = aby::EBackend::VULKAN
    }, {800, 600});
   
    aby::ui::Style canvas_style = {
        .background = aby::ui::Background {
            .color = {0.15f, 0.15f, 0.15f, 1.f },
            .texture = {}
        },
        .border = {
            .color = { 0.1f, 0.1f, 0.1f, 1.f },
            .width = 2.f
        }
    };
    aby::ui::Transform image_transform{
        .anchor = {
            .position = aby::ui::EAnchor::BOTTOM_MIDDLE,
            .offset = { 0, 0 }
        },
        .position = { 0, 0 },
        .size     = { 200, 100 },
    };
    aby::ui::ButtonStyle dark_mode_button = {
         .hovered  = { { 0.25f, 0.25f, 0.25f, 1.f }, {} }, // Slightly lighter gray
         .pressed  = { { 0.10f, 0.10f, 0.10f, 1.f }, {} }, // Even darker gray for a "pressed" effect
         .released = { { 0.15f, 0.15f, 0.15f, 1.f }, {} }, // Dark gray background
         .border   = { { 0.8f, 0.8f, 0.8f, 0.5f }, 2.0f }  // Light gray border for contrast
    };

    auto canvas = aby::ui:: Canvas::create(canvas_style);
    std::string button_text = "Options";
    auto button = aby::ui::Button::create(aby::ui::Transform{ .position = { 0, 0 }, .size = { 100, 30 } }, dark_mode_button, button_text);
    canvas->add_child(button);
    app.add_object(canvas);
    app.window()->set_maximized(true);
    auto font = aby::Font::create(&app.ctx(), "C:/Windows/fonts/arialbd.ttf", 18pt);
    app.run();
    return 0;
}