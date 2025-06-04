#pragma once
#include "Core/Window.h"

namespace aby::sys::posix {
    
    class Window final : public aby::Window {
    public:
        Window(const WindowInfo& info);
        ~Window();

        void* native() const override;
    private:
    };

}