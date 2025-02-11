#include <iostream>
#include <filesystem>
#include "Core/App.h"
#include "Core/Log.h"
#include "Core/Serialize.h"
#include "Rendering/Viewport.h"
#include "Core/Resource.h"
#include <fstream>


int main(int argc, char* argv[]) {
    aby::App app({ "Abyss", { 0, 1, 0 }, true }, {800, 600});
    auto obj = aby::create_ref<aby::Viewport>(app.window()->size());
    aby::Context& ctx = app.ctx();
    app.add_object(obj);
    app.run();
    return 0;
}