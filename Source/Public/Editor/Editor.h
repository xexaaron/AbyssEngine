#pragma once
#include "Core/App.h"
#include "Core/Resource.h"
#include "Rendering/Font.h"
#include "Platform/Platform.h"
#include "Platform/imgui/imtheme.h"
#include "Platform/imgui/imconsole.h"
#include "Utility/Delegate.h"
#include <filesystem>

namespace aby::editor {

    enum class ESettingsPage {
        NONE = 0,
        THEME,
        FONTS,
    };

    struct Settings {
        ESettingsPage current_page;
        imgui::Theme  current_theme;
        bool          show_settings;
        bool          show_console;
    };

    struct Icons {
        Resource minimize;
        Resource maximize;
        Resource exit;
        Resource plus;
    };

    class Editor  {
    public:
        Editor();

        aby::App& app();
    private:
        static aby::App create_app();
    private:
        aby::App m_App;
    };

    class EditorUI : public Object {
    public:
        EditorUI(App* app);
        void on_create(App* app, bool) override;
        void on_tick(App* app, Time deltatime) override;
        void on_event(App* app, Event& event) override;
        void on_destroy(App* app) override;

    private:
        void draw_dockspace();
        void draw_menubar();
        void draw_settings();
        void draw_settings_category(std::string_view name, ESettingsPage type);
        void draw_theme_settings();
        void draw_font_settings();
    private:
        App*     m_App;
        Icons    m_Icons;
        Settings m_Settings;
        imgui::Console m_Console;
    };

}