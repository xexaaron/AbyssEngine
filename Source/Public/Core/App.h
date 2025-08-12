#pragma once

#include "Core/Common.h"
#include "Core/Object.h"
#include "Core/Plugin.h"
#include "Rendering/Context.h"
#include "Rendering/Renderer.h"
#include <filesystem>

namespace aby {

	class App {
	public:
		explicit App(const AppInfo& app_info = {}, const WindowInfo& window_info = {});
		~App();

		void run();
		void quit();
		void restart();

		void set_name(const std::string& name);
		void add_object(Ref<Object> obj);
		void remove_object(Ref<Object> obj);
        void register_plugin(const fs::path& plugin);
        void remove_plugin(const std::string& plugin_name);
		void register_event(const std::function<void(Event&)>& event);
        template <typename T>
        void register_event(T* obj, void(T::* event)(Event&)) {
            m_Window->register_event([obj, event](Event& e) {
                return (obj->*event)(e);
            });
        }
		
		Dockspace&		 dockspace();
		const Dockspace& dockspace() const;
		Window*			 window();
		Window*			 window() const;
		Context&		 ctx();	
		const Context&   ctx() const;
		Renderer&		 renderer();
		const Renderer&  renderer() const;
		std::span<Ref<Object>> objects();
		std::span<const Ref<Object>> objects() const;
		const AppInfo& info() const;
		const std::string& name() const;
		const AppVersion&  version() const;

		fs::path    	cache();
		fs::path    	bin();
		const fs::path& exe();
	protected:
		void on_event(Event& event);
	private:
		friend std::vector<std::string> setup(int argc, char** argv);
#ifdef _WIN32
		friend void set_console_attached(App* app, bool attached_from_parent);
		bool bConsoleAttached;
#endif
	private:
		static fs::path m_ExePath;
		AppInfo         m_Info;
		Unique<Window>  m_Window;
		Ref<Context>    m_Ctx;
		Ref<Renderer>   m_Renderer;
		Ref<Dockspace>  m_Dockspace;
		std::vector<Ref<Object>> m_Objects;
		std::vector<LoadedPlugin> m_Plugins;
	};

}