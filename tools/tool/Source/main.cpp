#include <filesystem>
#include <CmdLine/CmdLine.h>
#include <PrettyPrint/PrettyPrint.h>
#include <map>
#include <iostream>
#include <algorithm>


// Added defintions to make it easier to view in editors that dont 
// recognize the cmake defined defintions.
#ifndef EXECUTABLE_FOLDER
#define EXECUTABLE_FOLDER "CMAKE_ERROR at tools/tool/CMakeLists.txt"
#endif
#ifndef EXECUTABLE_SUFFIX
#define EXECUTABLE_SUFFIX "CMAKE_ERROR at tools/tool/CMakeLists.txt"
#endif
#ifndef EXECUTABLE_FOLDER_NAME
#define EXECUTABLE_FOLDER_NAME "CMAKE_ERROR at tools/tool/CMakeLists.txt"
#endif
#ifndef EDITOR_EXECUTABLE_NAME
#define EDITOR_EXECUTABLE_NAME "CMAKE_ERROR at tools/tool/CMakeLists.txt"
#endif

namespace aby {

    void clean(const std::filesystem::path& path) {
        if (!std::filesystem::exists(path)) {
            std::cerr << "[Tool] [Error] Directory does not exist: " << path.string() << std::endl;
            return;
        }
        std::string confirm;
        std::cout << "[Tool] [Info] Are you sure you want to delete: " << path.string() << std::endl << "[Tool] [Info] Enter (y)es or (n)o" << std::endl;
        std::cin >> confirm;

        std::transform(confirm.begin(), confirm.end(), confirm.begin(), [](unsigned char c) { 
            return static_cast<char>(std::tolower(static_cast<int>(c))); 
        });

        if (confirm == "yes" || confirm == "y") {
            std::filesystem::remove_all(path);
        }
        else if (confirm == "no" || confirm == "n") {
            return;
        }
        else {
            std::cout << "[Tool] [Error] Invalid input. Exiting..." << std::endl;
        }
    }

    std::filesystem::path exec_dir(const char* argv0) {
        std::filesystem::path root(argv0);
        auto exec_root = root.parent_path();
        auto root_pp_fn = root.parent_path().filename();
        if (root_pp_fn != EXECUTABLE_FOLDER) {
            exec_root /= std::filesystem::path(EXECUTABLE_FOLDER_NAME) / EXECUTABLE_FOLDER;
        }
        if (!std::filesystem::exists(exec_root)) {
            exec_root = root.parent_path(); // Use same directory if not in packaged mode
        }
        return exec_root;
    }

    std::vector<std::string> get_tool_names(std::filesystem::path& exec_root) {
        std::vector<std::string> tool_names;
        if (!std::filesystem::exists(exec_root)) {
            aby::util::pretty_print(std::format("Tool directory does not exist {}", exec_root.string()), "Tool", aby::util::Colors{.box = aby::util::EColor::RED});
            return {};
        }
        for (auto& entry : std::filesystem::directory_iterator(exec_root)) {
            if (!entry.is_directory()) {
                auto path = entry.path();
                if ((!path.has_extension()) || (path.extension() == ".exe" && path.filename() != EDITOR_EXECUTABLE_NAME) &&
                    !path.filename().replace_extension().string().ends_with("Test") &&
                    path.filename().replace_extension().string() != "tool") 
                {
                    tool_names.push_back(path.filename().replace_extension().string());
                }
            }
        }
        tool_names.push_back("clean");
        return tool_names;
    }

}

int main(int argc, char** argv) {
    aby::util::CmdLine cmd;
    aby::util::CmdLine::Opts opts{
        .desc = "Run an AbyssEngine tool from the root directory",
        .name = "Tool",
        .cerr = std::cerr, 
        .help = true,
        .term_colors = true,
        .log_cmd = true,
    };

    auto exec_root = aby::exec_dir(argv[0]);
    std::vector<std::string> tool_names = aby::get_tool_names(exec_root);
    std::string args;

    std::map<std::string, bool> tools_to_run;
    for (const auto& tool_name : tool_names) {
        tools_to_run[tool_name] = false;
        std::string desc("Run " + tool_name + ".");
        if (tool_name == "clean") desc = "Clear cached files (fonts, shaders, etc.).";
        cmd.flag(tool_name, desc, &tools_to_run[tool_name], true, tool_names);
    }

    if (!cmd.opt("args", "Pass arguments to selected tool (quote enclosed).", &args, false)
        .parse(argc, argv, opts))
    {
        std::cout << '\n';
        return 1;
    }


    if (tools_to_run["clean"]) {
        aby::clean(exec_root / "Cache");
        return 0;
    }

    for (auto& tool : tools_to_run) {
        if (!tool.second) continue;
        if (tool.first == "clean") continue;
        
        std::filesystem::path tool_path = std::filesystem::absolute(exec_root / std::string(tool.first).append(EXECUTABLE_SUFFIX));
        if (!std::filesystem::exists(tool_path)) {
            aby::util::pretty_print(std::format("Tool does not exist: {}", tool_path.string()), "Tool", aby::util::Colors{.box = aby::util::EColor::RED});
            return 1;
        }
        std::string sys_cmd = tool_path.string() + " " + args;
        std::system(sys_cmd.c_str());
    }

    return 0;
}
