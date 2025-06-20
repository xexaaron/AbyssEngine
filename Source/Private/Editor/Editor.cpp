#include "Editor/Editor.h"
#include <Windows.h>
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

    EditorUI::EditorUI(App* app) : m_App(app) {
		
	}
	
	void EditorUI::on_create(App* app, bool) {
		auto path	   = app->bin() / "Textures";
		m_MinimizeIcon = Texture::create(&app->ctx(), path / "MinimizeIcon.png");
		m_MaximizeIcon = Texture::create(&app->ctx(), path / "MaximizeIcon.png");
		m_ExitIcon	   = Texture::create(&app->ctx(), path / "ExitIcon.png");
	}

    void EditorUI::on_tick(App* app, Time deltatime) {
		draw_dockspace();
		ImGui::Begin("Viewport");
		ImGui::End();
    }

	void EditorUI::draw_dockspace() {
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen) {
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		//if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		draw_menubar();

		ImGui::End();
	}

	void EditorUI::draw_menubar() {
		if (!ImGui::BeginMenuBar()) return;

		// === Menus ===
		if (ImGui::BeginMenu("Options")) {
			if (ImGui::MenuItem("Settings")) {
				// settings
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Restart")) {
				m_App->restart();
			}
			if (ImGui::MenuItem("Exit", "alt+f4")) {
				m_App->quit();
			}
			ImGui::EndMenu();
		}



		// === Buttons ===
		constexpr float button_dim   = 18.0f;
		constexpr float button_count = 3.f;
		constexpr float padding      = 5.f;
		constexpr float bttn_width   = (button_count + 1) * button_dim;

		auto  button_size = ImVec2(button_dim, button_dim);
		float right_edge  = ImGui::GetWindowContentRegionMax().x;
		auto& textures    = m_App->ctx().textures();
		auto  minimize    = textures.at(m_MinimizeIcon);
		auto  maximize    = textures.at(m_MaximizeIcon);
		auto  exit        = textures.at(m_ExitIcon);

		ImGui::SetCursorPosX(right_edge - bttn_width - padding);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		if (ImGui::ImageButton("Minimize", minimize->imgui_id(), button_size)) {
			m_App->window()->set_minimized(true);
		}
		ImGui::SameLine(0.0f);
		if (ImGui::ImageButton("Maximize", maximize->imgui_id(), button_size)) {
			m_App->window()->set_maximized(!m_App->window()->is_maximized());
		}
		ImGui::SameLine(0.0f);
		if (ImGui::ImageButton("Exit", exit->imgui_id(), button_size)) {
			m_App->quit();
		}
		ImGui::PopStyleVar();

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
		{
			m_App->window()->begin_drag();
		}

		ImGui::EndMenuBar();
	}

}