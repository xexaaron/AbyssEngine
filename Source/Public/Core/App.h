#pragma once

#include "Core/Common.h"
#include "Core/Object.h"
#include "Rendering/Context.h"
#include "Rendering/Renderer.h"
#include <glm/glm.hpp>
#include <iostream>
#include <filesystem>

namespace aby {

	class App {
	public:
		App(const AppInfo& app_info = {}, const WindowInfo& window_info = {});
		~App();

		void run();

		void set_name(const std::string& name);
		void add_object(Ref<Object> obj);
		void remove_object(Ref<Object> obj);

		Window*			window();
		Window*			window() const;
		Context&		ctx();	
		const Context&  ctx() const;
		Renderer&		renderer();
		const Renderer& renderer() const;
		std::span<Ref<Object>> objects();
		std::span<const Ref<Object>> objects() const;
		const AppInfo& info() const;
		const std::string& name() const;
		const AppVersion&  version() const;

		static fs::path    	   cache();
		static fs::path    	   bin();
		static const fs::path& exe();
	protected:
		void on_event(Event& event);
	private:
		static fs::path s_Path;
		AppInfo       m_Info;
		Unique<Window> m_Window;
		Ref<Context>  m_Ctx;
		Ref<Renderer> m_Renderer;
		std::vector<Ref<Object>> m_Objects;
	};


}