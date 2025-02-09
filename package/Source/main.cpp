#include <iostream>
#include <filesystem>
#include <tuple>
#include <vector>

#ifdef NDEBUG
    #define LIB_SUFFIX ""
    #define BUILD_MODE "Release"
#else
    #define LIB_SUFFIX "d"
    #define BUILD_MODE "Debug"
#endif

namespace aby {

    static std::tuple<bool,
        std::filesystem::path,
        std::filesystem::path,
        std::filesystem::path,
        std::filesystem::path,
        std::filesystem::path,
        std::filesystem::path
    > get_project_paths(const std::string& root_path) {
        namespace fs = std::filesystem;
        fs::path proj_root(root_path);
        if (!fs::exists(proj_root)) {
            std::cerr << "[Packager] [Error] Project root directory does not exist: " << proj_root << std::endl;
            return { false, proj_root, {}, {}, {}, {}, {} };
        }

        fs::path vendor_dir = proj_root / "Vendor";
        if (!fs::exists(vendor_dir)) {
            std::cerr << "[Packager] [Error] Vendor directory does not exist: " << vendor_dir << std::endl;
            return { false, proj_root, vendor_dir, {}, {}, {}, {} };
        }

        fs::path source_dir = proj_root / "Source";
        if (!fs::exists(source_dir)) {
            std::cerr << "[Packager] [Error] Source directory does not exist: " << source_dir << std::endl;
            return { false, proj_root, vendor_dir, source_dir, {}, {}, {} };
        }

        fs::path build_dir = proj_root / "build";
        if (!fs::exists(build_dir)) {
            fs::create_directories(build_dir);
        }

        fs::path vulkan_dir = vendor_dir / "VulkanSDK";
        if (!fs::exists(vulkan_dir)) {
            std::cerr << "[Packager] [Error] Vulkan vendor directory does not exist: " << vulkan_dir << std::endl;
            return { false, proj_root, vendor_dir, source_dir, build_dir, vulkan_dir, {} };
        }

        fs::path vulkan_bin_dir = vulkan_dir / "bin";
        if (!fs::exists(vulkan_bin_dir)) {
            std::cerr << "[Packager] [Error] VulkanSDK binary directory does not exist: " << vulkan_bin_dir << std::endl;
            return { false, proj_root, vendor_dir, source_dir, build_dir, vulkan_dir, vulkan_bin_dir };
        }

        return { true, proj_root, vendor_dir, source_dir, build_dir, vulkan_dir, vulkan_bin_dir };
    }

    static bool copy_dlls(const std::filesystem::path& build_dir, const std::filesystem::path& vulkan_bin_dir) {
        std::string shaderc_dll = "shaderc_shared" LIB_SUFFIX ".dll";
        auto shaderc_dll_in = std::filesystem::absolute(vulkan_bin_dir / shaderc_dll);
        auto shaderc_dll_out = build_dir / BUILD_MODE / shaderc_dll;

        // Check if the file exists and copy
        if (std::filesystem::exists(shaderc_dll_in)) {
            std::filesystem::copy_file(shaderc_dll_in, shaderc_dll_out, std::filesystem::copy_options::overwrite_existing);
            std::cout << "[Packager] [Info] Copied \"" << shaderc_dll_in.string() << "\" to \"" << shaderc_dll_out.string() << "\"" << std::endl;
        }
        else {
            std::cerr << "[Packager] [Error] Shaderc DLL does not exist: " << shaderc_dll_in.string() << std::endl;
            return false;
        }

        return true;
    }

    static bool copy_resource_dir(const std::filesystem::path& type, const std::filesystem::path& source_dir, const std::filesystem::path& build_dir) {
        std::filesystem::path resource_dir_in(source_dir / type);
        if (!std::filesystem::exists(resource_dir_in)) {
            std::cerr << "[Packager] [Error] Resource directory does not exist: " << resource_dir_in.string() << std::endl;
            return false;
        }

        std::filesystem::path resource_dir_out(build_dir / BUILD_MODE / type);
        if (std::filesystem::exists(resource_dir_in)) {
            std::filesystem::copy(resource_dir_in, resource_dir_out, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
            std::cout << "[Packager] [Info] Copied \"" << resource_dir_in.string() << "\" to \"" << resource_dir_out.string() << "\"" << std::endl;
        }
        else {
            std::cerr << "[Packager] [Error] Failed to copy resource directory" << std::endl;
            return false;
        }

        return true;
    }

    static bool copy_resources(const std::filesystem::path& source_dir, const std::filesystem::path& build_dir) {
        if (!copy_resource_dir("Shaders", source_dir, build_dir)) {
            return false;
        }
        if (!copy_resource_dir("Textures", source_dir, build_dir)) {
            return false;
        }

        return true;
    }

    static void clean_cache(const std::filesystem::path& build_dir) {
        auto cache_dir = build_dir / BUILD_MODE / "Cache";
        if (std::filesystem::exists(cache_dir)) {
            std::filesystem::remove_all(cache_dir);
            std::cout << "[Packager] [Info] Removed all files in Cache directory" << std::endl;
        }
        else {
            std::cerr << "[Packager] [Warn] No Cache existed to be removed" << std::endl;
        }
    }

    static void print_usage(bool desc = false) {
        std::cerr << "[Packager] [Usage] ./Packager <project_root_dir> [OPTIONAL]..." << std::endl;
        std::cerr << "  Options: " << std::endl;
        std::cerr << "   <project_root_dir> : Directory containing ./Vendor ./Source/Textures, and ./Source/Shaders" << std::endl;
        std::cerr << "   --clean            : Clear the binary file cache (<bin>/Cache)" << std::endl;
        std::cerr << "   --only-copy=<...>  : Only copy files then return. possible values [ shaders, textures, dlls, all ]" << std::endl;
        if (desc) {
            std::cerr << "  Description:" << std::endl;
            std::cerr << "   1: Package all resource files in <project_root_dir>/Source/<resource>" << std::endl;
            std::cerr << "   into the <bin>/<resource> directory." << std::endl;
            std::cerr << "   2: Copy necessary dll files to the <bin> directory." << std::endl;
        }
    }

}

int main(int argc, char** argv) {
    if (argc < 2) { 
        aby::print_usage(true);
        return 2;
    }
    std::vector<std::string> args(argc);
    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name
        std::string arg = argv[i];
        std::transform(arg.begin(), arg.end(), arg.begin(), [](char c) {
            return std::tolower(c);
        });
        args[i - 1] = arg;
    }
    bool clean = false;
    bool only_copy[4] = { false, false, false, false };
    for (const auto& arg : args) { 
        if (arg == "-h" || arg == "-help" || arg == "--h" || arg == "--help") {
            aby::print_usage(true);
            return 2;
        }
        else if (arg == "--clean") {
            clean = true;
        }
        else if (arg.starts_with("--only-copy=")) {
            if (arg.contains(",")) {
                if (arg.contains("textures")) {
                    only_copy[0] = true;
                } 
                if (arg.contains("shaders")) {
                    only_copy[1] = true;
                }
                if (arg.contains("dlls")) {
                    only_copy[2] = true;
                }
                if (only_copy[0] && only_copy[1] && only_copy[2]) {
                    only_copy[3] = true;
                }
                else {
                    aby::print_usage();
                    std::cerr << "[Packager] [Error] Invalid option: " << arg << std::endl;
                    return 2;
                }
            }
            else {
                if (arg.ends_with("textures")) {
                    only_copy[0] = true;
                }
                else if (arg.ends_with("shaders")) {
                    only_copy[1] = true;
                }
                else if (arg.ends_with("dlls")) {
                    only_copy[2] = true;
                }
                else if (arg.ends_with("all")) {
                    only_copy[3] = true;
                }
                else {
                    aby::print_usage();
                    std::cerr << "[Packager] [Error] Invalid option: " << arg << std::endl;
                    return 2;
                }
            }
        }
    }

    auto [
          result, proj_root, vendor_dir, source_dir,
          build_dir, vulkan_dir, vulkan_bin_dir 
    ] = aby::get_project_paths(argv[1]);


    if (!result) {
        return 1;
    }

    if (only_copy[3]) {
        if (!aby::copy_resources(source_dir, build_dir)) {
            return 1;
        }
        if (!aby::copy_dlls(build_dir, vulkan_bin_dir)) {
            return 1;
        }
        return 0;
    }
    else if (only_copy[0] || only_copy[1] || only_copy[2]) {
        if (only_copy[0]) {
            aby::copy_resource_dir("Textures", source_dir, build_dir);
        }
        if (only_copy[1]) {
            aby::copy_resource_dir("Shaders", source_dir, build_dir);
        } 
        if (only_copy[2]) {
            aby::copy_dlls(build_dir, vulkan_bin_dir);
        }
        return 0;
    }
    else {
        if (!aby::copy_resources(source_dir, build_dir)) {
            return 1;
        }
        if (!aby::copy_dlls(build_dir, vulkan_bin_dir)) {
            return 1;
        }
    }



    if (clean) {
        aby::clean_cache(build_dir);
    }
    else {
        std::cout << "[Packager] [Info] Skipping Cache deletion" << std::endl;
    }

    std::cout << "[Packager] [Status] Finished" << std::endl;
    return 0;
}