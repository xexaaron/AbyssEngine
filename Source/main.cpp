#include <iostream>
#include <filesystem>
#include "Core/App.h"
#include "Core/Log.h"
#include "Core/Serialize.h"
#include "Rendering/Viewport.h"
#include "Core/Resource.h"
#include "Rendering/UI/Canvas.h"
#include <fstream>


int main(int argc, char* argv[]) {
    aby::App app(aby::AppInfo{ 
        .Name     = "Abyss",
        .Version  = aby::AppVersion{ 
            .Major = 0,
            .Minor = 1,
            .Patch = 0
        },
        .bInherit = true,
        .Backend  = aby::EBackend::VULKAN
    }, {800, 600});
    
    // auto viewport = aby::create_ref<aby::Viewport>(app.window()->size());
    
    aby::ui::Style canvas_style{
        .bg = aby::ui::Background{ 
            .color   = { 0.9, 0.9, 0.9 },
            .texture = {} //aby::Texture::create(&app.ctx(), app.bin() / "Textures/DefaultTexture.png")
        },
        .border = aby::ui::Border{ 
            .color = { 0.1, 0.1, 0.1 },
            .width = 4.f    
        }
    };

    auto canvas = aby::ui::Canvas::create(app.window()->size(), canvas_style);
    app.add_object(canvas);
    app.run();
    
    return 0;
}