#pragma once
#include "Core/App.h"

namespace aby {

    struct Icons {
        Resource minimize;
        Resource maximize;
        Resource exit;
    };

    class Dockspace final : public Object {
    public:
        void on_create(App* app, bool deserialized);
		void on_event(App* app, Event& event);
		void on_tick(App* app, Time deltatime);
		void on_destroy(App* app);
    private:
        void draw_dockspace(App* app);
        void draw_menubar(App* app);
    private:
        Icons m_Icons;
    };


}