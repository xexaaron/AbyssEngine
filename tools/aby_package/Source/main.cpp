#include <CmdLine/CmdLine.h>
#include <filesystem>

#ifndef EXECUTABLE_FOLDER
#define EXECUTABLE_FOLDER "CMAKE_ERROR at tools/package/CMakeLists.txt"
#endif
#ifndef EXECUTABLE_SUFFIX
#define EXECUTABLE_SUFFIX "CMAKE_ERROR at tools/package/CMakeLists.txt"
#endif
#ifndef EXECUTABLE_FOLDER_NAME
#define EXECUTABLE_FOLDER_NAME "CMAKE_ERROR at tools/package/CMakeLists.txt"
#endif

namespace aby {
    std::filesystem::path exec_dir(const char* argv0, const std::string& build_mode) {
        std::filesystem::path root(argv0);
        auto exec_root = root.parent_path();
        auto root_pp_fn = root.parent_path().filename();
        if (root_pp_fn != build_mode) {
            exec_root /= std::filesystem::path(EXECUTABLE_FOLDER_NAME) / build_mode;
        }
        return exec_root;
    }

    bool skip_file_or_dir(const std::filesystem::directory_entry& entry) {
        auto path_str = entry.path().string();
        return path_str.contains("test")       || 
            path_str.contains("Test")          || 
            entry.path().extension() == ".lib" ||
            entry.path().extension() == ".pdb" ||
            entry.path().filename().replace_extension("") == "aby_package";
    }
}

int main(int argc, char** argv) {
    aby::util::CmdLine cmd;
    aby::util::CmdLine::Opts opts{
        .desc        = "Package binaries into folder",
        .name        = "Package",
        .cerr        = std::cerr,
        .help        = true,
        .term_colors = true,
        .log_cmd     = true,
    };

    std::string output_dir = "./bin";
    std::string build_mode = EXECUTABLE_FOLDER;

    if (!cmd.opt("o", "Output directory (default: " + output_dir + ")",  &output_dir, false)
        .opt("b", "build mode (default: " + std::string(EXECUTABLE_FOLDER) + ")", &build_mode, false)
        .parse(argc, argv, opts))
    {
        return 1;
    }

    if (!std::filesystem::exists(output_dir)) {
        std::filesystem::create_directory(output_dir);
    } else {
        std::filesystem::remove_all(output_dir);
        std::filesystem::create_directory(output_dir);
    }

    auto source_dir = aby::exec_dir(argv[0], build_mode);
    auto dir_iter = std::filesystem::recursive_directory_iterator(source_dir);

#ifdef _WIN32
    std::filesystem::create_directories(std::filesystem::path(output_dir) / "Lib");
#endif

    for (const auto& entry : dir_iter) {
        if (aby::skip_file_or_dir(entry)) continue;

        auto relative_path = std::filesystem::relative(entry.path(), source_dir);
        auto target_path = std::filesystem::path(output_dir) / relative_path;

        try {
            if (entry.is_directory()) {
                std::filesystem::create_directories(target_path);
            } else if (entry.is_regular_file()) {
                std::filesystem::create_directories(target_path.parent_path());
                std::filesystem::copy_file(entry.path(), target_path, std::filesystem::copy_options::overwrite_existing);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << "\n";
        }
    }

    return 0;
}