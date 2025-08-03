#include "Core/Plugin.h"
#include "Core/App.h"

namespace aby {

    LoadedPlugin::LoadedPlugin(Unique<sys::SharedLibrary>&& lib, Plugin* plugin) :
        lib(std::move(lib)),
        plugin(plugin) 
    {

    }

    LoadedPlugin::operator bool() const {
        return lib && plugin;
    }
    

    Plugin::Plugin(const std::string& name) : m_Name(name) {}

    Plugin::~Plugin() { }

    LoadedPlugin Plugin::load(const fs::path& plugin) {
        auto lib = sys::SharedLibrary::create(plugin);
        if (!lib->load()) return LoadedPlugin(nullptr, nullptr);

        auto fn = reinterpret_cast<Plugin*(*)()>(lib->load_fn("create_plugin"));
        if (!fn) return LoadedPlugin(nullptr, nullptr);

        Plugin* module = fn();
        return LoadedPlugin(std::move(lib), module);
    }

    const std::string& Plugin::name() const {
        return m_Name;
    }

}