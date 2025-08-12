#include "Rendering/Dockspace.h"

namespace aby {

    void Dockspace::on_create(App* app, bool deserialized) {
		auto path	     = app->bin() / "Textures";
        m_Icons.minimize = Texture::create(&app->ctx(), path / "MinimizeIcon.png");
		m_Icons.maximize = Texture::create(&app->ctx(), path / "MaximizeIcon.png");
		m_Icons.exit     = Texture::create(&app->ctx(), path / "ExitIcon.png");
    }
   
    void Dockspace::on_event(App* app, Event& event) {

    }

    void Dockspace::on_tick(App* app, Time deltatime) {
	    draw_dockspace(app);
    }
    
    void Dockspace::on_destroy(App* app) {

    }

    void Dockspace::draw_dockspace(App* app) {
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
		float min_win_size_x = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0), dockspace_flags);
		}

		style.WindowMinSize.x = min_win_size_x;

		draw_menubar(app);

		ImGui::End();
    }

    void Dockspace::draw_menubar(App* app) {
        if (!ImGui::BeginMenuBar()) return;

		// === Menus ===
		if (ImGui::BeginMenu("Options")) {
			if (ImGui::MenuItem("Restart", "alt+f5")) {
				app->restart();
			}
			if (ImGui::MenuItem("Exit", "alt+f4")) {
				app->quit();
			}

			ImGui::EndMenu();
		}

		for (auto& menu : m_Menus) {
			if (ImGui::BeginMenu(menu.name.c_str())) {
				for (auto item : menu.items) {
					if (ImGui::MenuItem(item.name.c_str(), item.shortcut.c_str())) {
						item.action();
					}
				}
				ImGui::EndMenu();
			}
			
		}

		// === Buttons ===
		constexpr float button_dim   = 18.0f;
		constexpr float button_count = 3.f;
		constexpr float padding      = 10.f;
		constexpr float bttn_width   = (button_count + 1) * button_dim;

		auto  button_size = ImVec2(button_dim, button_dim);
		float right_edge  = ImGui::GetWindowContentRegionMax().x;
		auto& textures    = app->ctx().textures();
		auto  minimize    = textures.at(m_Icons.minimize);
		auto  maximize    = textures.at(m_Icons.maximize);
		auto  exit        = textures.at(m_Icons.exit);


		ImGui::SetCursorPosX(right_edge - bttn_width - padding);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		if (ImGui::ImageButton("Minimize", minimize->imgui_id(), button_size)) {
			app->window()->set_minimized(true);
		}
		ImGui::SameLine(0.0f);
		if (ImGui::ImageButton("Maximize", maximize->imgui_id(), button_size)) {
			app->window()->set_maximized(!app->window()->is_maximized());
		}
		ImGui::SameLine(0.0f);
		if (ImGui::ImageButton("Exit", exit->imgui_id(), button_size)) {
			app->quit();
		}
		ImGui::PopStyleVar();

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
		{
			app->window()->begin_drag();
		}

		ImGui::EndMenuBar();

    }

	void Dockspace::add_menu(const Menu& menu) {
		m_Menus.push_back(menu);
	}
	
	void Dockspace::remove_menu(const std::string& menu_name) {
		auto it = std::remove_if(m_Menus.begin(), m_Menus.end(), [&menu_name](const Menu& menu){
			return menu_name == menu.name;
		})
		if (it != m_Menus.end()) {
			m_Menus.erase(it);
		} else {
			ABY_WARN("Tried to remove menu but it does not exist: {}", menu_name);
		}
	}	


}