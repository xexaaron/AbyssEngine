#include <filesystem>
#include <CmdLine/CmdLine.h>
#include <map>

#ifndef NDEBUG
#define EXECUTABLE_FOLDER "Debug"
#else
#define EXECUTABLE_FOLDER "Release"
#endif

#ifdef _WIN32
#define EXECUTABLE_SUFFIX ".exe"
#else
#define EXECUTABLE_SUFFIX
#endif

void clean(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        std::cerr << "[Tool] [Error] Directory does not exist: " << path.string() << std::endl;
        return;
    }
    std::string confirm;
    std::cout << "[Tool] [Info] Are you sure you want to delete: " << path.string() << std::endl << "[Tool] [Info] Enter (y)es or (n)o" << std::endl;
    std::cin >> confirm;

    std::transform(confirm.begin(), confirm.end(), confirm.begin(), ::tolower);

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

int main(int argc, char** argv) {
    aby::util::CmdLine cmd;
    aby::util::CmdLine::Opts opts{
        .desc = "Run an AbyssEngine tool from the root directory",
        .cerr = std::cerr, 
        .help = true
    };

    std::string tool;
    std::string args;
    if (!cmd.opt("run", "Select a tool to run [localize, tests, watchdog, clean]", &tool, true)
        .opt("args", "Pass arguments to selected tool", &args, false)
        .parse(argc, argv, opts))
    {
        return 1;
    }

    if (tool.empty()) {
        std::cerr << "[Tool] [Error] Argument for tool was empty" << std::endl;
        return 1;
    }

    std::filesystem::path root(argv[0]);
    auto exec_root = root.parent_path() / "build" / EXECUTABLE_FOLDER;

    if (tool == "clean") {
        clean(exec_root / "Cache");
        return 0;
    }

    std::filesystem::path tool_path = std::filesystem::absolute(exec_root / tool.append(EXECUTABLE_SUFFIX));
    if (!std::filesystem::exists(tool_path)) {
        std::cerr << "[Tool] [Error] Tool does not exist: " << tool_path.string() << std::endl;
    }

    std::string sys_cmd = tool_path.string();
    sys_cmd.append(" ");
    sys_cmd.append(args);
    int result = std::system(sys_cmd.c_str());
    if (result != 0) {
        std::cerr << "[Tool] [Error] Tool exited with code " << result << std::endl;
    }

    return 0;
}