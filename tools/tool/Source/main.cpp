#include <filesystem>
#include <CmdLine/CmdLine.h>
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

std::string get_tool_names(const std::filesystem::path& exec_root) {
    std::string tool_names = "[";
    bool first_tool = true;
    for (auto& entry : std::filesystem::directory_iterator(exec_root)) {
        if (!entry.is_directory()) {
            auto path = entry.path();
            if ((!path.has_extension()) || (path.extension() == ".exe" && path.filename() != EDITOR_EXECUTABLE_NAME)) {
                if (!first_tool) {
                    tool_names.append(", ");
                }
                tool_names.append(path.filename().replace_extension("").string());
                first_tool = false;
            }
        }
    }
    if (!first_tool) {
        tool_names.append(", ");
    }
    tool_names.append("clean");
    tool_names.append("]");
    return tool_names;
}

int main(int argc, char** argv) {
    aby::util::CmdLine cmd;
    aby::util::CmdLine::Opts opts{
        .desc = "Run an AbyssEngine tool from the root directory",
        .name = "Tool",
        .cerr = std::cerr, 
        .help = true,
        .term_colors = true,
    };

    std::filesystem::path root(argv[0]);
    auto exec_root = root.parent_path();
    auto root_pp_fn = root.parent_path().filename();
    if (root_pp_fn != EXECUTABLE_FOLDER) {
        exec_root /= std::filesystem::path(EXECUTABLE_FOLDER_NAME) / EXECUTABLE_FOLDER;
    }
    std::string tool_names = get_tool_names(exec_root);
    std::string description = "Select a tool to run " + tool_names;

    std::string tool;
    std::string args;
    if (!cmd.opt("run", description, &tool, true)
        .opt("args", "Pass arguments to selected tool", &args, false)
        .parse(argc, argv, opts))
    {
        std::cout << '\n';
        return 1;
    }

    if (tool.empty()) {
        std::cerr << "[Tool] [Error] Argument for tool was empty" << std::endl;
        std::cout << '\n';
        return 1;
    }

    if (tool == "clean") {
        clean(exec_root / "Cache");
        return 0;
    }

    std::filesystem::path tool_path = std::filesystem::absolute(exec_root / tool.append(EXECUTABLE_SUFFIX));
    if (!std::filesystem::exists(tool_path)) {
        std::cerr << "[Tool] [Error] Tool does not exist: " << tool_path.string() << std::endl;
        std::cout << '\n';
        return 1;
    }

    // Create and execute the system command
    std::string sys_cmd = tool_path.string() + " " + args;
    int result = std::system(sys_cmd.c_str());
    if (result != 0) {
        std::cerr << "[Tool] [Error] Tool exited with code " << result << std::endl;
    }

    std::cout << '\n';

    return 0;
}
