#pragma once

#ifdef _WIN32

#include "Platform/SharedLibrary.h"
#include <Windows.h>

namespace aby::sys::win32 {

    class SharedLibrary : public sys::SharedLibrary {
    public:
        SharedLibrary(const fs::path& path);
        ~SharedLibrary() = default;

        bool  load()    override;
        void  unload()  override;
        void* load_fn(const std::string& name) override;
    private:
        HMODULE m_Dll;
    };

}

#endif