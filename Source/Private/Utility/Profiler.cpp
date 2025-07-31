#include "Utility/Profiler.h"
#include "Utility/File.h"
#include "Core/App.h"
#include "Core/Log.h"

namespace aby::util {

	ProfileResult::ProfileResult(const std::string& label, const std::source_location& source) : 
		label(label),
		source(source),
		duration(0.f),
		m_Start(std::chrono::high_resolution_clock::now())
	{

	}
	
	ProfileResult::~ProfileResult() {
		duration = std::chrono::duration<float>(
			std::chrono::high_resolution_clock::now() - m_Start
		).count();
		Profiler::get().profile(*this);
	}

	Profiler& Profiler::get() {
		static Profiler profiler;
		return profiler;
	}

	void Profiler::set_app(App* app) {
		m_App = app;
	}

	void Profiler::profile(const ProfileResult& result) {
		ABY_ASSERT(m_App, "App must be set to use Profiler");
		fs::path path = m_App->cache() / "Profiler";
		if (!fs::exists(path)) 
			fs::create_directories(path);
		std::ofstream ofs(path / "Profiler.csv", std::ios::app);
		if (ofs.is_open()) {
			std::string data = std::format("{:%T}, {}, [{}], {}, {}\n",
				std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()),
				result.source.file_name(),
				result.source.function_name(),
				result.label,
				result.duration.milli()
			);
			ofs << data;
			ofs.close();
		}
	}

}