#pragma once
#include <imgui/imgui.h>

namespace aby::imgui {


    class Theme {
    public:
        Theme(const std::string& name);
        Theme(const std::string& name, const std::filesystem::path& dir);
        Theme(const std::string& name, ImGuiStyle* style);
        
        static bool exists(const std::string& name, const std::filesystem::path& dir); 

        void save(const std::filesystem::path& dir);
        void reset();
        void set_current();

        ImGuiStyle* style();
        std::string& name();
    private:
        std::string m_Name;
        ImGuiStyle m_Style;
    };

}
