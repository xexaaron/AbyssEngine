#pragma once
#include "Core/Common.h"
#include <ostream>
#include <thread>
#include <filesystem>
#include <functional>
#include <mutex>


namespace aby::sys {

	auto is_terminal(const std::ostream& os) -> bool;
	auto set_thread_name(std::thread& thread, const std::string& name) -> bool;
	auto get_exec_path() -> fs::path;
    auto set_cursor(ECursor cursor) -> bool;
	auto get_pid() -> int;

}