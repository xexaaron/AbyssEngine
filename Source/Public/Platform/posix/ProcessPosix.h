#pragma once
#include "Platform/Process.h"

namespace aby::sys::posix {

	class Process : public sys::Process {
	public:
		Process(const std::function<void(const std::string&)>& read_callback);
		~Process();

		bool is_open() override;
		bool open(const std::string& cmd) override;
		void close() override;
		void write(const std::string& data) override;
		void kill() override;
	private:
	};
}
