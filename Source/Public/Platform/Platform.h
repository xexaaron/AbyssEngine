#pragma once
#include "Core/Common.h"
#include <thread>
#include <filesystem>
#include <functional>
#include <string>

namespace aby {
	class App;
}

namespace aby::sys {

	enum class EFileType {
		ANY = 0,
		FILE,
		FOLDER,
	};

	auto is_terminal(const std::ostream& os) -> bool;
	auto set_thread_name(std::thread& thread, const std::string& name) -> bool;
	auto get_exec_path() -> fs::path;
	auto get_pid() -> int;
	auto get_args(int& argc, char**& argv) -> void;
	auto free_args(int argc, char** argv) -> void;
	auto glfw_to_platform_keycode(int glfw_key) -> int;
	auto get_last_err() -> std::string;
	/**
	* Open the file dialog to select a file/folder.
	* Cannot be called before aby::Window has been initialized.
	* 
	* @param app	   Pointer to the application.
	* @param start_dir Directory for the file dialog picker to start in,
	* @param filter	   File types to be selected only.
	* 
	* @return file path to selected file. Empty if failed.
	*/
	auto open_file_dialog(App* app, const fs::path& start_dir = "", EFileType filter = EFileType::ANY) -> fs::path; 


}