#include <iostream>
#include <filesystem>
#include "Core/App.h"
#include "Core/Log.h"
#include "Core/Serialize.h"
#include "Rendering/Viewport.h"
#include "Core/Resource.h"
#include "Rendering/UI/UI.h"
#include "Rendering/Font.h"
#include "Platform/Platform.h"
#include <fstream>

namespace editor {

    aby::App create_app() {
        const aby::AppInfo app_info{
            .name = "Abyss",
            .version = aby::AppVersion{
                .major = 0,
                .minor = 1,
                .patch = 0
            },
            .binherit = true,
            .backend = aby::EBackend::VULKAN
        };
        const aby::WindowInfo window_info{
            .size  = { 800, 600 },
            .flags = aby::EWindowFlags::MAXIMIZED | 
                     aby::EWindowFlags::VSYNC
        };
        return aby::App(app_info, window_info);
    }
    
}

int main(int argc, char* argv[]) {
    using namespace aby;
    
    auto app          = editor::create_app();
    auto canvas_style = ui::Style::dark_mode();
    auto button_style = ui::ButtonStyle::dark_mode();
    button_style.border.width = 0.f;
    auto canvas       = ui::Canvas::create(canvas_style);

    aby::ui::TextInfo opts_info{
        .text  = "Options",
        .color = { 1, 1, 1, 1 },
        .scale = 1.f,
        .alignment = aby::ui::ETextAlignment::CENTER,
    };
    aby::ui::TextInfo other_info = opts_info;
    other_info.text = "Other";

    auto button       = ui::Button::create(ui::Transform{ .position = { 0, 0 }, .size = { 100, 30 } }, button_style, opts_info);
    auto button1      = ui::Button::create(ui::Transform{ .position = { 0, 0 }, .size = { 100, 30 } }, button_style, other_info);
    auto container    = ui::LayoutContainer::create(
        ui::Transform{ 
            .anchor = {},
            .position = {0, 0},
            .size = { app.window()->size().x, 30 } 
        },
        aby::ui::Style{
            .background = canvas_style.background,
            .border     = { .color = { 0.8f, 0.8f, 0.8f, 0.5f }, .width = 2.0f }
        },
        ui::EDirection::HORIZONTAL,
        ui::ELayout::AUTO,
        2.0f
    );
    auto font = Font::create(&app.ctx(), App::bin() / "C:/Windows/Fonts/arialbd.ttf", 14);
    auto console_style = ui::Style::dark_mode();
    console_style.background.color *= 0.9;
    auto console = ui::Console::create(console_style);
    container->add_child(button);
    container->add_child(button1);
    canvas->add_child(container);
    canvas->add_child(console);
    app.add_object(canvas);
    app.run();
    return 0;
}