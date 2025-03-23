#pragma once
#include "Core/Common.h"
#include <functional>
#include <string>

namespace aby::sys {

	class Process {
	public:
		static Unique<Process> create(const std::function<void(const std::string&)>& read_callback);
		virtual ~Process() = default;

		virtual bool open(const std::string& cmd) = 0;
		virtual bool is_open() = 0;
		virtual void close() = 0;
		virtual void write(const std::string& data) = 0;
		virtual void kill() = 0;
	protected:
		explicit Process(const std::function<void(const std::string&)>& read_callback);
	protected:
		std::function<void(const std::string&)> m_Read;
	};
}