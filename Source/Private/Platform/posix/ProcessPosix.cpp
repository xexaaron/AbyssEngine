#include "Platform/posix/ProcessPosix.h"

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
#define POSIX 
#endif

#ifdef POSIX

namespace aby::sys::posix {

	Process::Process(const std::function<void(const std::string&)>& read_callback) :
		sys::Process(read_callback) 
	{

	}

	Process::~Process() {

	}

	bool Process::is_open() {

	}


	bool Process::open(const std::string& cmd) {

	}

	void Process::close() {

	}
	void Process::write(const std::string& data) {

	}

	void Process::read() {

	}

	void Process::kill() override {

	}

}

#endif
