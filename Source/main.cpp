#include <filesystem>
#include "Core/App.h"
#include "Rendering/Viewport.h"
#include "Core/Resource.h"
#include "Rendering/UI/UI.h"
#include "Rendering/Font.h"
#include "Platform/Platform.h"

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
            .backend  = aby::EBackend::VULKAN
        };
        const aby::WindowInfo window_info{
            .size  = glm::u32vec2{ 800, 600 },
            .flags = aby::EWindowFlags::MAXIMIZED | 
                     aby::EWindowFlags::VSYNC
        };
        return aby::App(app_info, window_info);
    }
    
    aby::Ref<aby::ui::Canvas> create_canvas(aby::App& app) {
        using namespace aby;
        if (!Font::create(&app.ctx(), App::bin() / "Fonts/IBM_Plex_Mono/IBMPlexMono-Bold.ttf", 12)) {
            return nullptr;
        }

        auto canvas_style = ui::Style::dark_mode();
        auto button_style = ui::ButtonStyle::dark_mode();
        auto canvas = ui::Canvas::create(canvas_style);

        ui::TextInfo opts_info{
            .text = "Options",
            .color = { 1, 1, 1, 1 },
            .scale = 1.f,
            .alignment = ui::ETextAlignment::CENTER,
        };
        ui::TextInfo other_info = opts_info;
        other_info.text = "Other";
        button_style.border.width = 0.f;

        auto button = ui::Button::create(ui::Transform{ .position = { 0, 0 }, .size = { 100, 30 } }, button_style, opts_info);
        auto button1 = ui::Button::create(ui::Transform{ .position = { 0, 0 }, .size = { 100, 30 } }, button_style, other_info);
        auto container = ui::LayoutContainer::create(
            ui::Transform{
                .anchor = {},
                .position = {0, 0},
                .size = { app.window()->size().x, 30 }
            },
            ui::Style{
                .background = canvas_style.background,
                .border = {.color = { 0.8f, 0.8f, 0.8f, 0.5f }, .width = 2.0f }
            },
            ui::EDirection::HORIZONTAL,
            ui::ELayout::AUTO,
            2.0f
        );
        auto console_style = ui::Style::dark_mode();
        console_style.background.color *= 0.9f;
        console_style.background.color.a = 1.f;

        auto console = ui::Console::create(console_style);
        container->add_child(button);
        container->add_child(button1);
        canvas->add_child(container);
        canvas->add_child(console);
        return canvas;
    }

}

aby::App& aby::main(const std::vector<std::string>&args) {
    static auto app = editor::create_app();
    auto canvas     = editor::create_canvas(app);
    app.add_object(canvas);
    return app;
}



