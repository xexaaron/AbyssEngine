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

    struct Icons {
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
        App*     m_App;
        imgui::Console m_Console;
    };

}