#pragma once
#include "Core/Common.h"
#include <thread>
#include <filesystem>
#include <functional>


namespace aby::sys {

	auto is_terminal(const std::ostream& os) -> bool;
	auto set_thread_name(std::thread& thread, const std::string& name) -> bool;
	auto get_exec_path() -> fs::path;
	auto get_pid() -> int;

}