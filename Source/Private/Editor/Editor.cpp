#include "Editor/Editor.h"
#include "Platform/imgui/imwidget.h"

#include <imgui/imgui_internal.h>
namespace aby::editor {

    Editor::Editor() : 
        m_App(Editor::create_app())
    {
		
        if (!aby::Font::create(&m_App.ctx(), m_App.bin() / "Fonts/IBM_Plex_Mono/IBMPlexMono-Bold.ttf", 12)) {
            throw std::runtime_error("Could not create Font!");
        }

		m_App.add_object(create_ref<EditorUI>(&m_App));
    }

    aby::App& Editor::app() {
        return m_App;
    }

    aby::App Editor::create_app() {
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

}


namespace aby::editor {

	EditorUI::EditorUI(App* app) :
		m_App(app),
		m_Console("Console", false)
	{

	}
	
	void EditorUI::on_create(App* app, bool) {
		auto path	 = app->bin() / "Textures";
		Logger::add_callback([&](const LogMsg& msg) {
			m_Console.add_msg(msg);
		});
	}

    void EditorUI::on_tick(App* app, Time deltatime) {
		if (ImGui::Begin("Viewport")) {

		}
		
		bool open_console = true;
		m_Console.draw(&open_console);

		ImGui::End();

    }

	void EditorUI::on_event(App* app, Event& event) {
	}

	void EditorUI::on_destroy(App* app) {
	}
	
}

namespace aby {
	App& main(const std::vector<std::string>& args) {
        static editor::Editor editor;
        return editor.app();
    }
}
