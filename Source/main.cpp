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

int main(int argc, char* argv[]) {
    using namespace aby;
    AppInfo app_info{
        .name = "Abyss",
        .version = AppVersion{
            .major = 0,
            .minor = 1,
            .patch = 0
        },
        .binherit = true,
        .backend = EBackend::VULKAN
    };

    App  app(app_info, {800, 600});
    auto canvas_style = ui::Style::dark_mode();
    auto button_style = ui::ButtonStyle::dark_mode();
    auto canvas       = ui::Canvas::create(canvas_style);
    auto button       = ui::Button::create(ui::Transform{ .position = { 0, 0 }, .size = { 100, 30 } }, button_style, "Options");
    auto button1      = ui::Button::create(ui::Transform{ .position = { 0, 0 }, .size = { 100, 30 } }, button_style, "Other");

    auto container    = ui::LayoutContainer::create(ui::Transform{ .position = { 0, 0 }, .size = { 100, 30 } }, canvas_style, ui::EDirection::HORIZONTAL);
    auto font         = Font::create(&app.ctx(), "C:/Windows/fonts/arialbd.ttf", 18);

    container->add_child(button);
    container->add_child(button1);
    canvas->add_child(container);
    app.add_object(canvas);
    app.window()->set_maximized(true);
    app.run();
    return 0;
}