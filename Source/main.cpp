#include <filesystem>
#include "Core/App.h"
#include "Core/Resource.h"
#include "Rendering/UI/UI.h"
#include "Rendering/Font.h"
#include "Platform/Platform.h"
#include "Utility/Delegate.h"
#include "Rendering/UI/WidgetSwitcher.h"

namespace editor {

    class Editor  {
    public:
        Editor() : 
            m_App(Editor::create_app()),
            m_Canvas(aby::ui::Canvas::create(aby::ui::ImageStyle::dark_mode()))
        {
            if (!aby::Font::create(&m_App.ctx(), m_App.bin() / "Fonts/IBM_Plex_Mono/IBMPlexMono-Bold.ttf", 12)) {
                throw std::runtime_error("Could not create Font!");
            }

            auto menubar = Editor::create_menubar(m_App, m_Canvas);
            auto switcher = aby::ui::WidgetSwitcher::create(aby::ui::Transform{}, aby::ui::ImageStyle::dark_mode());

            m_Canvas->add_child(menubar);
            m_Canvas->add_child(switcher);
            auto console = Editor::create_console();
            console->set_anchor(aby::ui::Anchor{ aby::ui::EAnchor::BOTTOM_LEFT, {} });
            m_Canvas->add_child(console);
            m_App.add_object(m_Canvas);
        }

        aby::App& app() {
            return m_App;
        }

        aby::Ref<aby::ui::Canvas> canvas() {
            return m_Canvas;
        }
    private:
        static aby::App create_app() {
            return aby::App(
                aby::AppInfo{
                   .name = "Abyss",
                   .version = aby::AppVersion{
                       .major = 0,
                       .minor = 1,
                       .patch = 0
                   },
                   .binherit = true,
                   .backend = aby::EBackend::VULKAN
                },
                aby::WindowInfo{
                    .size = glm::u32vec2{ 800, 600 },
                    .flags = aby::EWindowFlags::MAXIMIZED |
                             aby::EWindowFlags::VSYNC
                }
            );
        }
        static aby::Ref<aby::ui::LayoutContainer> create_menubar(aby::App& app, aby::Ref<aby::ui::Canvas> canvas) {

            auto button_style = aby::ui::ButtonStyle::dark_mode();

            aby::ui::TextInfo opts_info{
                .text = "Options",
                .color = { 1, 1, 1, 1 },
                .scale = 1.f,
                .alignment = aby::ui::ETextAlignment::CENTER,
            };
            aby::ui::TextInfo other_info = opts_info;
            other_info.text = "Other";
            button_style.border.width = 0.f;

            auto button = aby::ui::Button::create(aby::ui::Transform{ .position = { 0, 0 }, .size = { 100, 30 } }, button_style, opts_info);
            auto button1 = aby::ui::Button::create(aby::ui::Transform{ .position = { 0, 0 }, .size = { 100, 30 } }, button_style, other_info);
            auto container = aby::ui::LayoutContainer::create(
                aby::ui::Transform{
                    .anchor = {},
                    .position = {0, 0},
                    .size = { app.window()->size().x, 30 }
                },
                aby::ui::ImageStyle::dark_mode(),
                aby::ui::EDirection::HORIZONTAL,
                aby::ui::ELayout::AUTO,
                2.0f
            );
            container->add_child(button);
            container->add_child(button1);
            return container;
        }
        static aby::Ref<aby::ui::Console> create_console() {
            auto console_style = aby::ui::ImageStyle::dark_mode();
            console_style.color *= 0.9f;
            console_style.color.a = 1.f;
            return aby::ui::Console::create(console_style);
        }
    private:
        aby::App m_App;
        aby::Ref<aby::ui::Canvas> m_Canvas;
    };
}

void add(int, int) {

}

aby::App& aby::main(const std::vector<std::string>& args) {
    static editor::Editor editor;
    return editor.app();
}