#pragma once
#include "Core/Common.h"
#include "Core/Object.h"
#include "Core/Resource.h"

namespace aby {

    class App;

    struct MenuItem {
        std::string name;
        std::string shortcut;
        std::function<void()> action;
    };

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
        void activate_menu_item(const std::string& menu, const std::string& item);
    private:
        void draw_dockspace(App* app);
        void draw_menubar(App* app);
    private:
        struct Icons {
            Resource minimize;
            Resource maximize;
            Resource exit;
        } m_Icons;
        std::vector<Menu> m_Menus;
    };


}