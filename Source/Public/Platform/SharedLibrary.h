#pragma once
#include "Core/Common.h"

namespace aby::sys {
    
    class SharedLibrary {
    public:
        static Unique<SharedLibrary> create(const fs::path& path); 
        virtual ~SharedLibrary() = default;

        virtual bool  load() = 0;
        virtual void  unload() = 0;
        virtual void* load_fn(const std::string& name) = 0;
    
        const fs::path& path() const;
    protected:
        SharedLibrary(const fs::path& path);
    private:
        fs::path m_Path;
    };


}