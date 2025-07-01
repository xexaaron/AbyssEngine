#include "Browser/Browser.h"
#include "Core/Log.h"
#include "Core/App.h"

#include <imgui/imgui.h>


namespace aby::web {

        Browser::Browser() : 
            m_App(nullptr)
        {

        }

        Browser::~Browser() {

        }

        void Browser::on_create(App* app, bool) {
            m_App = app;
        }
        
        void Browser::on_destroy(App* app) {

        }
        
        void Browser::on_tick(App* app, Time dt) {

        }
        
        void Browser::on_event(App* app, Event& event) {

        }

}