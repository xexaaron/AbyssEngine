#pragma once

#ifdef _WIN32

#include "Platform/Process.h"
#include "Platform/win32/PlatformWin32.h"
#include "Core/Thread.h"
#include <queue>

namespace aby::sys::win32 {

	struct IOHandles {
		HANDLE read  = INVALID_HANDLE_VALUE;
		HANDLE write = INVALID_HANDLE_VALUE;
	};

	struct ProcessHandles {
		IOHandles in   = {};	  // input  streams
		IOHandles out  = {};	  // output streams
		HPCON	  con  = nullptr; // Console handle
		HANDLE	  proc = INVALID_HANDLE_VALUE;
	};

	class Process : public sys::Process {
	public:
		explicit Process(const std::function<void(const std::string&)>& read_callback);
		~Process() final;

		bool open(const std::string& cmd) override;
		void close() override;
		void write(const std::string& data) override;
		bool is_open() override;
		void kill() override;
	private:
		void loop();
		std::vector<std::string> split(const std::string& cmd);
		void create_console();
		STARTUPINFOEX create_startup_info();
		void sanitize(std::string& buffer);
	private:
		ProcessHandles			m_Handles;
		std::queue<std::string> m_Writes;
		Unique<Thread>			m_Thread;
		std::atomic<bool>		bRunning;
	};
}

#endif