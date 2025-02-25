#pragma once

#include "Core/Common.h"
#include <cstdio>
#include <thread>
#include <string>

namespace aby::sys::posix {

	auto is_terminal(std::FILE* stream) -> bool;
	auto set_thread_name(std::thread& thread, const std::string& name) -> bool;
	auto get_exec_path() -> fs::path;
	auto set_cursor(ECursor cursor) -> bool;
	auto get_pid() -> int;

}