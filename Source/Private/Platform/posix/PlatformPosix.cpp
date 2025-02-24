#include "Platform/posix/PlatformPosix.h"


#ifdef POSIX

#include <unistd.h>
#include <pthread.h>

namespace aby::sys::posix {

	auto is_terminal(std::FILE* stream) -> bool {
		return isatty(fileno(stream));
	}

	auto set_thread_name(std::thread& thread, const std::string& name) -> bool {
		auto handle = reinterpret_cast<pthread_t>(thread.native_handle());
		return (pthread_setname_np(handle, name.c_str()) == 0);
	}

	auto get_exec_path() -> fs::path {
		char path_buff[PATH_MAX];
		ssize_t count = readlink("/proc/self/exe", path_buff, PATH_MAX);
		if (count == -1) {
			perror("readlink");
			return "";
		}
		return fs::path(std::string(path_buff, count));
	}


}

#endif