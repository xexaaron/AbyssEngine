#pragma once
#include "Core/App.h"
#include "Utility/Delegate.h"
namespace aby {

    struct Icons {
        Resource minimize;
        Resource maximize;
        Resource exit;
    };

    struct MenuItem {
        std::string name;
        std::string shortcut;
        std::function<void()> action;
    }

    struct Menu {
        std::string name;
        std::vector<MenuItem> items;
    };



    class Dockspace final : public Object {
    public:
        void on_create(App* app, bool deserialized);
		void on_event(App* app, Event& event);
		void on_tick(App* app, Time deltatime);
		void on_destroy(App* app);
    
        void add_menu(const Menu& menu);
        void remove_menu(const std::string& menu_name);
    private:
        void draw_dockspace(App* app);
        void draw_menubar(App* app);
    private:
        Icons m_Icons;
        std::vector<Menu> m_Menus;
    };


}