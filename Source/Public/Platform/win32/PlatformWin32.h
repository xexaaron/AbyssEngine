#pragma once

#include "Core/Common.h"
#include <cstdio>
#include <Windows.h>
#include <iostream>
#include <thread>

#define WIN32_CHECK(x) do { \
    HRESULT r = (x); \
    if (FAILED(r)) { \
        ABY_ASSERT(false, "[win32] API call failed with HRESULT: 0x{:X}", r); \
    } \
} while(0)

namespace aby::sys::win32 {

    auto to_string(const std::wstring& wstring) -> std::string;
    auto to_wstring(const std::string& string) -> std::wstring;
	auto is_terminal(std::FILE* stream) -> bool;
    auto set_thread_name(std::thread& thread, const std::string& name) -> bool;
    auto get_exec_path() -> fs::path;
    auto set_cursor(ECursor cursor) -> bool;
    auto get_pid() -> int;

}