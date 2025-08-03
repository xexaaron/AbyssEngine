#ifdef _WIN32

#include "Platform/win32/SharedLibraryWin32.h"
#include "Core/Log.h"

namespace aby::sys::win32 {


    SharedLibrary::SharedLibrary(const fs::path& path) :
        sys::SharedLibrary(path) {}

    bool SharedLibrary::load()  {
        m_Dll = LoadLibraryA(path().string().c_str());
        if (!m_Dll) {
            ABY_ERR("Failed to load dll: {}", path());
            return false;
        }
        return true;
    }
    
    void SharedLibrary::unload()  {
        FreeLibrary(m_Dll);
    }

    void* SharedLibrary::load_fn(const std::string& name) {
        void* fn = GetProcAddress(m_Dll, name.c_str());
        if (!fn) { 
            ABY_ERR("Failed to GetProcAddress of {} from {}", name, this->path());
            this->unload();
            return nullptr;
        }
        return fn;
    }

    

}

#endif