#pragma once
#include "Core/Event.h"
#include "Core/Time.h"
#include "Platform/SharedLibrary.h"

#define ABY_REGISTER_PLUGIN(T, ...) extern "C" __declspec(dllexport) aby::Plugin* create_plugin() { return new T(__VA_ARGS__); }

namespace aby {
    
    class App;
    class Plugin;

    struct LoadedPlugin {
        LoadedPlugin(Unique<sys::SharedLibrary>&& lib, Plugin* plugin);
        LoadedPlugin(const LoadedPlugin&) = default;
        LoadedPlugin(LoadedPlugin&&) noexcept = default;

        explicit operator bool() const;
        LoadedPlugin& operator=(const LoadedPlugin&) = default;
        LoadedPlugin& operator=(LoadedPlugin&&) noexcept = default;

        Unique<sys::SharedLibrary> lib = nullptr;
        Plugin* plugin                 = nullptr;
    };

    class Plugin {
    public:
        Plugin(const std::string& name);
        Plugin(const Plugin&) = default;
        Plugin(Plugin&&) noexcept = default;
        virtual ~Plugin();

        /**
         * Load a plugin
         * @param plugin Filepath to shared library that implements plugin interface.
         * @return Plugin* (null if unsuccesful)    
         */
        static LoadedPlugin load(const fs::path& plugin);

        virtual void on_load(App* app) = 0;
        virtual void on_unload(App* app) = 0;
        virtual void on_tick(App* app, Time deltatime) = 0;
        virtual void on_event(App* app, Event& event) = 0;
        
        const std::string& name() const;
    private:
        std::string m_Name;
    };

    /**
     * @brief Each plugin needs to define this function to be loaded. 
     * @return new Plugin
     */
    extern "C" __declspec(dllexport)
    Plugin* create_plugin();

};