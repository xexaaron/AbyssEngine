#pragma once
#include "Core/App.h"
#include "Core/Resource.h"
#include "Widget/UI.h"
#include "Rendering/Font.h"
#include "Platform/Platform.h"
#include "Utility/Delegate.h"
#include "Widget/WidgetSwitcher.h"  

#include <filesystem>

#include <imgui/imgui.h>

namespace aby::editor {

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
        void draw_dockspace();
        void draw_menubar();
    private:
        App* m_App;
        Resource m_MinimizeIcon;
        Resource m_MaximizeIcon;
        Resource m_ExitIcon;
    };

}