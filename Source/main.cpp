#include "Core/App.h"
#include "Core/Resource.h"
#include "Widget/UI.h"
#include "Rendering/Font.h"
#include "Platform/Platform.h"
#include "Utility/Delegate.h"
#include "Widget/WidgetSwitcher.h"
#include <filesystem>

namespace editor {

    class Editor  {
    public:
        Editor() : 
            m_App(Editor::create_app())
        {
            if (!aby::Font::create(&m_App.ctx(), m_App.bin() / "Fonts/IBM_Plex_Mono/IBMPlexMono-Bold.ttf", 12)) {
                throw std::runtime_error("Could not create Font!");
            }
        }

        aby::App& app() {
            return m_App;
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
                    .flags = //aby::EWindowFlags::MAXIMIZED |
                             aby::EWindowFlags::VSYNC
                }
            );
        }
    private:
        aby::App m_App;
    };
}

aby::App& aby::main(const std::vector<std::string>& args) {
    static editor::Editor editor;
    return editor.app();
}