#include "Editor/Editor.h"

namespace aby {

    App& main(const std::vector<std::string>& args) {
        static editor::Editor editor;
        return editor.app();
    }

}