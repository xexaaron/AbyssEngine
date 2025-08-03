#include "Core/Plugin.h"
#include "Core/Log.h"

namespace aby {

    class MyPlugin : public Plugin {
    public:
        MyPlugin() : Plugin("MyPlugin") {}
        ~MyPlugin() = default;

        void on_load(App* app) override { 
            ABY_LOG("MyPlugin Loaded");
        }
        void on_unload(App* app) override { 
            ABY_LOG("MyPlugin Unloaded");
        }
        void on_tick(App* app, Time deltatime) override { 
            ABY_LOG("MyPlugin Tick");
        }
        void on_event(App* app, Event& event) override { 
            ABY_LOG("MyPlugin OnEvent");
        }
    private:
    };

}

ABY_REGISTER_PLUGIN(aby::MyPlugin);
