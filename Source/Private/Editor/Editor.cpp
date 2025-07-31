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
		m_Icons{},
		m_Settings{
			.current_page  = ESettingsPage::NONE,
			.current_theme = imgui::Theme("Default"),
			.show_settings = false, 
			.show_console  = false,
		},
		m_Console("Console", false),
		m_Browser(app->window())
	{

	}
	
	void EditorUI::on_create(App* app, bool) {
		auto path	     = app->bin() / "Textures";
		m_Icons.minimize = Texture::create(&app->ctx(), path / "MinimizeIcon.png");
		m_Icons.maximize = Texture::create(&app->ctx(), path / "MaximizeIcon.png");
		m_Icons.exit     = Texture::create(&app->ctx(), path / "ExitIcon.png");
		m_Icons.plus	 = Texture::create(&app->ctx(), path / "PlusIcon.png");
		Logger::add_callback([&](const LogMsg& msg) {
			m_Console.add_msg(msg);
		});
		m_Browser.on_create(app, false);
	}

    void EditorUI::on_tick(App* app, Time deltatime) {
		draw_dockspace();
		draw_settings();
		m_Browser.on_tick(app, deltatime);

		if (ImGui::Begin("Viewport")) {

		}
		
		m_Console.draw(&m_Settings.show_console);

		ImGui::End();

		//ImGui::ShowStyleEditor();
    }

	void EditorUI::on_event(App* app, Event& event) {
		m_Browser.on_event(app, event);
	}

	void EditorUI::on_destroy(App* app) {
		m_Browser.on_destroy(app);
	}

	void EditorUI::draw_settings() {
		if (!m_Settings.show_settings) return;
		if (!ImGui::Begin("Settings", &m_Settings.show_settings)) {
			ImGui::End();
			return;
		}

		ImVec2 size   = ImGui::GetWindowSize();
		float  width  = size.x;
		float  category_width = size.x * 0.2f;
		float  settings_width = width - category_width - 22.5f;

		if (ImGui::BeginChild("Categories", ImVec2(category_width, 0), ImGuiChildFlags_Border)) {
			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;
			if (ImGui::CollapsingHeader("Appearance", node_flags)) {
				draw_settings_category("Theme", ESettingsPage::THEME);
				draw_settings_category("Fonts", ESettingsPage::FONTS);
			}
		}
		ImGui::EndChild();

		ImGui::SameLine();

		if (ImGui::BeginChild("Settings", ImVec2(settings_width, 0.f), ImGuiChildFlags_Border)) {
			switch (m_Settings.current_page) {
				case ESettingsPage::THEME: draw_theme_settings(); break;
				case ESettingsPage::FONTS: draw_font_settings();  break;
				case ESettingsPage::NONE:						  break;
				default:										  break;
			}
		}
		ImGui::EndChild();

		ImGui::End();
	}

	void EditorUI::draw_settings_category(std::string_view name, ESettingsPage type) {
		ImGuiTreeNodeFlags selected = m_Settings.current_page == type ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None;
		if (ImGui::TreeNodeEx(name.data(), selected | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth)) {
			if (ImGui::IsItemClicked()) {
				m_Settings.current_page = type;
			}
			ImGui::TreePop();
		}
	}

	void EditorUI::draw_theme_settings() {
		auto theme_dir = m_App->cache() / "Themes";
		// if (ImGui::BeginCombo("##Theme", curr_theme_name)) {
		// 	auto it = std::filesystem::directory_iterator(theme_dir);
		// 	for (auto& theme : it) {
		// 		auto name = theme.path().filename().replace_extension("").string();
		// 		if (ImGui::Selectable(name.c_str(), curr_theme_name == name)) {
		// 			imgui::Theme theme(name, theme_dir);
		// 			theme.set_current();
		// 			m_Settings.current_theme = theme;
		// 		}
		// 	}
		// 	ImGui::EndCombo();
		// }
		
		bool disable_opts = true; // m_Settings.current_theme.name().starts_with("Default");


		ImGui::SameLine();
		auto plus = m_App->ctx().textures().at(m_Icons.plus);
		auto minus = m_App->ctx().textures().at(m_Icons.minimize);
		if (ImGui::ImageButton("AddTheme", plus->imgui_id(), ImVec2(16, 16))) {
			auto it = std::filesystem::directory_iterator(theme_dir);
			std::size_t new_themes = 0;
			for (auto& theme : it) {
				auto name = theme.path().filename().replace_extension("").string();
				if (name.starts_with("NewTheme")) {
					new_themes++;
				}
			}
			std::string new_theme_name = new_themes == 0 ? "NewTheme" : "NewTheme" + std::to_string(new_themes);
			imgui::Theme new_theme(new_theme_name, m_Settings.current_theme.style());
			new_theme.set_current();
			m_Settings.current_theme = new_theme;
			new_theme.save(theme_dir);
		}
		ImGui::SameLine();
		ImGui::BeginDisabled(disable_opts);
		if (ImGui::ImageButton("DeleteTheme", minus->imgui_id(), ImVec2(16, 16))) {
			std::filesystem::remove(m_App->cache() / "Themes" / (m_Settings.current_theme.name() + ".imtheme"));
			m_Settings.current_theme = imgui::Theme("Default", m_App->cache() / "Themes");
			m_Settings.current_theme.set_current();
		}
		ImGui::EndDisabled();

		ImGui::Separator();

		ImGui::BeginDisabled(disable_opts);

		ImGuiStyle& style = *m_Settings.current_theme.style();
		bool rebuild = false;
		
		char name_buff[128];
		strncpy(name_buff, m_Settings.current_theme.name().c_str(), sizeof(name_buff));
		ImGui::SeparatorText("Name");
		if (ImGui::InputText("##Name", name_buff, sizeof(name_buff))) {
			m_Settings.current_theme.name() = std::string(name_buff);
			rebuild = true;
		}
		ImGui::Separator();
		if (ImGui::CollapsingHeader("Colors")) {
			for (int i = 0; i < ImGuiCol_COUNT; i++) {
				rebuild |= ImGui::ColorEdit4(ImGui::GetStyleColorName(i), (float*)&style.Colors[i], ImGuiColorEditFlags_Float);
			}
		}
		if (ImGui::CollapsingHeader("Options")) {
			ImGui::SeparatorText("Opacity");
			rebuild |= imgui::InputFloat("Alpha", style.Alpha, { 0.1f, 1.f, 1.f });
			rebuild |= imgui::InputFloat("Disabled Alpha", style.DisabledAlpha, { 0.f, 1.f, 0.6f });
			ImGui::SeparatorText("Windows");
			rebuild |= imgui::InputVec2("Window Padding", style.WindowPadding);
			rebuild |= ImGui::InputFloat("Window Rounding", &style.WindowRounding);
			rebuild |= ImGui::InputFloat("Window Border Size", &style.WindowBorderSize);
			rebuild |= ImGui::InputFloat("Window Border Hover Padding", &style.WindowBorderHoverPadding);
			rebuild |= ImGui::InputFloat2("Window Min Size", (float*)&style.WindowMinSize);
			rebuild |= ImGui::InputFloat2("Window Title Align", (float*)&style.WindowTitleAlign);
			rebuild |= ImGui::Combo("Window Menu Button Position", (int*)&style.WindowMenuButtonPosition, "None\0Left\0Right\0Up\0Down\0");
			ImGui::SeparatorText("Child Windows");
			rebuild |= ImGui::InputFloat("Child Rounding", &style.ChildRounding);
			rebuild |= ImGui::InputFloat("Child Border Size", &style.ChildBorderSize);
			ImGui::SeparatorText("Popups");
			rebuild |= ImGui::InputFloat("Popup Rounding", &style.PopupRounding);
			rebuild |= ImGui::InputFloat("Popup Border Size", &style.PopupBorderSize);
			ImGui::SeparatorText("Frames");
			rebuild |= ImGui::InputFloat2("Frame Padding", (float*)&style.FramePadding);
			rebuild |= ImGui::InputFloat("Frame Rounding", &style.FrameRounding);
			rebuild |= ImGui::InputFloat("Frame Border Size", &style.FrameBorderSize);
			ImGui::SeparatorText("Items");
			rebuild |= ImGui::InputFloat2("Item Spacing", (float*)&style.ItemSpacing);
			rebuild |= ImGui::InputFloat2("Item Inner Spacing", (float*)&style.ItemInnerSpacing);
			rebuild |= ImGui::InputFloat2("Cell Padding", (float*)&style.CellPadding);
			rebuild |= ImGui::InputFloat2("Touch Extra Padding", (float*)&style.TouchExtraPadding);
			rebuild |= ImGui::InputFloat("Indent Spacing", &style.IndentSpacing);
			rebuild |= ImGui::InputFloat("Columns Min Spacing", &style.ColumnsMinSpacing);
			rebuild |= ImGui::InputFloat("Scrollbar Size", &style.ScrollbarSize);
			rebuild |= ImGui::InputFloat("Scrollbar Rounding", &style.ScrollbarRounding);
			rebuild |= ImGui::InputFloat("Grab Min Size", &style.GrabMinSize);
			rebuild |= ImGui::InputFloat("Grab Rounding", &style.GrabRounding);
			rebuild |= ImGui::InputFloat("Log Slider Deadzone", &style.LogSliderDeadzone);
			rebuild |= ImGui::InputFloat("Image Border Size", &style.ImageBorderSize);
			ImGui::SeparatorText("Tabs");
			rebuild |= ImGui::InputFloat("Tab Rounding", &style.TabRounding);
			rebuild |= ImGui::InputFloat("Tab Border Size", &style.TabBorderSize);
			rebuild |= ImGui::InputFloat("Tab Close Button Width (Selected)", &style.TabCloseButtonMinWidthSelected);
			rebuild |= ImGui::InputFloat("Tab Close Button Width (Unselected)", &style.TabCloseButtonMinWidthUnselected);
			rebuild |= ImGui::InputFloat("Tab Bar Border Size", &style.TabBarBorderSize);
			rebuild |= ImGui::InputFloat("Tab Bar Overline Size", &style.TabBarOverlineSize);
			ImGui::SeparatorText("Tables");
			rebuild |= ImGui::InputFloat("Table Angled Headers Angle", &style.TableAngledHeadersAngle);
			rebuild |= ImGui::InputFloat2("Table Angled Headers Text Align", (float*)&style.TableAngledHeadersTextAlign);
			ImGui::SeparatorText("Trees");
			rebuild |= ImGui::InputFloat("Tree Lines Size", &style.TreeLinesSize);
			rebuild |= ImGui::InputFloat("Tree Lines Rounding", &style.TreeLinesRounding);
			ImGui::SeparatorText("Buttons & Combos");
			rebuild |= ImGui::Combo("Color Button Position", (int*)&style.ColorButtonPosition, "None\0Left\0Right\0Up\0Down\0");
			rebuild |= ImGui::InputFloat2("Button Text Align", (float*)&style.ButtonTextAlign);
			rebuild |= ImGui::InputFloat2("Selectable Text Align", (float*)&style.SelectableTextAlign);
			ImGui::SeparatorText("Separators");
			rebuild |= ImGui::InputFloat("Separator Text Border Size", &style.SeparatorTextBorderSize);
			rebuild |= ImGui::InputFloat2("Separator Text Align", (float*)&style.SeparatorTextAlign);
			rebuild |= ImGui::InputFloat2("Separator Text Padding", (float*)&style.SeparatorTextPadding);
			ImGui::SeparatorText("Display");
			rebuild |= ImGui::InputFloat2("Display Window Padding", (float*)&style.DisplayWindowPadding);
			rebuild |= ImGui::InputFloat2("Display Safe Area Padding", (float*)&style.DisplaySafeAreaPadding);
			rebuild |= ImGui::InputFloat("Docking Separator Size", &style.DockingSeparatorSize);
			rebuild |= ImGui::InputFloat("Mouse Cursor Scale", &style.MouseCursorScale);
			ImGui::SeparatorText("Anti-Aliasing");
			rebuild |= ImGui::Checkbox("Anti-Aliased Lines", &style.AntiAliasedLines);
			rebuild |= ImGui::Checkbox("Anti-Aliased Lines (Tex)", &style.AntiAliasedLinesUseTex);
			rebuild |= ImGui::Checkbox("Anti-Aliased Fill", &style.AntiAliasedFill);
			rebuild |= ImGui::InputFloat("Curve Tessellation Tolerance", &style.CurveTessellationTol);
			rebuild |= ImGui::InputFloat("Circle Tessellation Max Error", &style.CircleTessellationMaxError);

			ImGui::SeparatorText("Hovers");
			rebuild |= ImGui::InputFloat("Hover Stationary Delay", &style.HoverStationaryDelay);
			rebuild |= ImGui::InputFloat("Hover Delay Short", &style.HoverDelayShort);
			rebuild |= ImGui::InputFloat("Hover Delay Normal", &style.HoverDelayNormal);
			if (ImGui::TreeNode("Hover Flags For Tooltip Mouse")) {
				rebuild |= ImGui::CheckboxFlags("Stationary", (int*)&style.HoverFlagsForTooltipMouse, ImGuiHoveredFlags_Stationary);
				rebuild |= ImGui::CheckboxFlags("No Delay", (int*)&style.HoverFlagsForTooltipMouse, ImGuiHoveredFlags_DelayNone);
				rebuild |= ImGui::CheckboxFlags("Short Delay", (int*)&style.HoverFlagsForTooltipMouse, ImGuiHoveredFlags_DelayShort);
				rebuild |= ImGui::CheckboxFlags("Normal Delay", (int*)&style.HoverFlagsForTooltipMouse, ImGuiHoveredFlags_DelayNormal);
				rebuild |= ImGui::CheckboxFlags("No Shared Delay", (int*)&style.HoverFlagsForTooltipMouse, ImGuiHoveredFlags_NoSharedDelay);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Hover Flags For Tooltip Nav")) {
				rebuild |= ImGui::CheckboxFlags("Stationary", (int*)&style.HoverFlagsForTooltipMouse, ImGuiHoveredFlags_Stationary);
				rebuild |= ImGui::CheckboxFlags("No Delay", (int*)&style.HoverFlagsForTooltipMouse, ImGuiHoveredFlags_DelayNone);
				rebuild |= ImGui::CheckboxFlags("Short Delay", (int*)&style.HoverFlagsForTooltipMouse, ImGuiHoveredFlags_DelayShort);
				rebuild |= ImGui::CheckboxFlags("Normal Delay", (int*)&style.HoverFlagsForTooltipMouse, ImGuiHoveredFlags_DelayNormal);
				rebuild |= ImGui::CheckboxFlags("No Shared Delay", (int*)&style.HoverFlagsForTooltipMouse, ImGuiHoveredFlags_NoSharedDelay);
				ImGui::TreePop();
			}

			if (rebuild) {
				m_Settings.current_theme.save(theme_dir);
				m_Settings.current_theme.set_current();
			}
		}

		ImGui::EndDisabled();
	}

	void EditorUI::draw_font_settings() {

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
			
			ImGui::MenuItem("Settings", "", &m_Settings.show_settings);

			ImGui::Separator();
			
			if (ImGui::MenuItem("Restart", "alt+f5")) {
				m_App->restart();
			}
			if (ImGui::MenuItem("Exit", "alt+f4")) {
				m_App->quit();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {

			ImGui::MenuItem("Console", "", &m_Settings.show_console);
			
			ImGui::Separator();
			
			ImGui::MenuItem("Settings", "", &m_Settings.show_settings);

			ImGui::EndMenu();
		}

		// === Buttons ===
		constexpr float button_dim   = 18.0f;
		constexpr float button_count = 3.f;
		constexpr float padding      = 10.f;
		constexpr float bttn_width   = (button_count + 1) * button_dim;

		auto  button_size = ImVec2(button_dim, button_dim);
		float right_edge  = ImGui::GetWindowContentRegionMax().x;
		auto& textures    = m_App->ctx().textures();
		auto  minimize    = textures.at(m_Icons.minimize);
		auto  maximize    = textures.at(m_Icons.maximize);
		auto  exit        = textures.at(m_Icons.exit);


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

namespace aby {
	App& main(const std::vector<std::string>& args) {
        static editor::Editor editor;
        return editor.app();
    }
}
