# Getting Started


## Cloning

```sh
git clone --branch master --recurse-submodules --shallow-submodules https://github.com/xexaaron/AbyssEngine.git
```

## Building

```bash
cmake -S ./AbyssEngine -B ./AbyssEngine/build 
cmake --build ./AbyssEngine/build
```

## Setup

```cpp
#include "Core/EntryPoint.h"

// Define external main function to create an application.
aby::App& aby::main(const std::vector<std::string>& args) {
    aby::AppInfo app_info{
        .name     = "App Name",
        .version  = { 1, 0, 0 }, 
        .binherit = true,        // Window inherits app name.
        .backend  = aby::EBackend::VULKAN
    };

    aby::WindowInfo window_info{
        .size     = { 800, 600 },
        .flags    = aby::EWindowFlags::NONE,
        .title    = "" // Inherit app name.
    };

    static aby::App app(app_info, window_info);
    return app;
}

```