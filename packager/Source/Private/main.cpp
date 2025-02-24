#include <iostream>
#include <filesystem>
#include <vector>
#include <format>

#ifdef NDEBUG
    #define LIB_SUFFIX ""
    #define BUILD_MODE "Release"
#else
    #define LIB_SUFFIX "d"
    #define BUILD_MODE "Debug"
#endif

#define return_if(x) if ((x)) { return EC_FAILURE; }

#define ABY_ERR(fmt, ...) std::cerr << std::format("[Packager] [Error] " fmt, __VA_ARGS__) << '\n';
#define ABY_LOG(fmt, ...) std::cout << std::format("[Packager] [Info]  " fmt, __VA_ARGS__) << '\n';

namespace fs = std::filesystem;

enum ErrorCode {
    EC_SUCCESS = 0,
    EC_FAILURE = 1,
    EC_USAGE = 2,
};

struct Args {
    ErrorCode ec;
    bool      clean;
    fs::path  root;
    fs::path  vendor;
    fs::path  source;
    fs::path  build;
    fs::path  vendor_build;
    fs::path  vulkan;
    fs::path  vulkan_bin;
};

static void print_usage(bool desc = false) {
    ABY_LOG("Usage: ./packager <project_root_dir> [OPTIONAL]...");
    ABY_LOG("  Options: ");
    ABY_LOG("   <project_root_dir> : Directory containing ./Vendor ./Source/Textures, and ./Source/Shaders");
    ABY_LOG("   --clean            : Clear the binary file cache (<bin>/Cache)");
    if (desc) {
        ABY_LOG("  Description:");
        ABY_LOG("   1: Package all resource files in <project_root_dir>/Source/<resource>");
        ABY_LOG("   into the <bin>/<resource> directory.");
        ABY_LOG("   2: Copy necessary dll files to the <bin> directory.");
    }
}

static Args parse_args(int argc, char** argv) {
    if (argc < 2) {
        print_usage(true);
        return { EC_USAGE };
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
    for (const auto& arg : args) {
        if (arg == "-h" || arg == "-help" || arg == "--h" || arg == "--help") {
            print_usage(true);
            return { EC_USAGE };
        }
        else if (arg == "--clean") {
            clean = true;
        }
    }
    fs::path proj_root(argv[1]);
    if (!fs::exists(proj_root)) {
        ABY_ERR("Project root directory does not exist: {}", proj_root.string());
        return { EC_FAILURE };
    }

    fs::path vendor_dir = proj_root / "Vendor";
    if (!fs::exists(vendor_dir)) {
        ABY_ERR("Vendor directory does not exist: {}", vendor_dir.string());
        return { EC_FAILURE };
    }

    fs::path source_dir = proj_root / "Source";
    if (!fs::exists(source_dir)) {
        ABY_ERR("Source directory does not exist: {}", source_dir.string());
        return { EC_FAILURE };
    }

    fs::path build_dir = proj_root / "build";
    if (!fs::exists(build_dir)) {
        fs::create_directories(build_dir);
    }

    fs::path vendor_build = build_dir / "Vendor";
    if (!fs::exists(vendor_build)) {
        ABY_ERR("Vendor build directory does not exist: {}", vendor_build.string());
        return { EC_FAILURE };
    }

    fs::path vulkan_dir = vendor_dir / "VulkanSDK";
    if (!fs::exists(vulkan_dir)) {
        ABY_ERR("Vulkan directory does not exist: {}", vulkan_dir.string());
        return { EC_FAILURE };
    }

    fs::path vulkan_bin_dir = vulkan_dir / "bin";
    if (!fs::exists(vulkan_bin_dir)) {
        ABY_ERR("Vulkan binary directory does not exist: {}", vulkan_bin_dir.string());
        return { EC_FAILURE };
    }



    return {
        .ec = EC_SUCCESS,
        .clean = clean,
        .root = proj_root,
        .vendor = vendor_dir,
        .source = source_dir,
        .build = build_dir,
        .vendor_build = vendor_build,
        .vulkan = vulkan_dir,
        .vulkan_bin = vulkan_bin_dir,
    };
}

static bool copy_dlls(const Args& args) {
    auto shaderc_dll = "shaderc_shared" LIB_SUFFIX ".dll";
    auto shaderc_dll_in = std::filesystem::absolute(args.vulkan_bin / shaderc_dll);
    auto shaderc_dll_out = args.build / BUILD_MODE / shaderc_dll;
    if (std::filesystem::exists(shaderc_dll_in)) {
        std::filesystem::copy_file(shaderc_dll_in, shaderc_dll_out, std::filesystem::copy_options::overwrite_existing);
        ABY_LOG("Copied \"{}\" to \"{}\"", shaderc_dll_in.string(), shaderc_dll_out.string());
    }
    else {
        ABY_ERR("Shaderc DLL does not exist: \"{}\"", shaderc_dll_in.string());
        return false;
    }
    {
        auto freetype_dll = "freetype" LIB_SUFFIX ".dll";
        auto freetype_dll_in = std::filesystem::absolute(args.vendor_build / "freetype" / BUILD_MODE / freetype_dll);
        auto freetype_dll_out = args.build / BUILD_MODE / freetype_dll;
        if (std::filesystem::exists(freetype_dll_in)) {
            std::filesystem::copy_file(freetype_dll_in, freetype_dll_out, std::filesystem::copy_options::overwrite_existing);
            ABY_LOG("Copied \"{}\" to \"{}\"", freetype_dll_in.string(), freetype_dll_out.string());
        }
        else {
            ABY_ERR("Freetype DLL does not exist: \"{}\"", freetype_dll_in.string());
            return false;
        }
    }

#ifndef NDEBUG
    auto freetype_pdb = "freetype" LIB_SUFFIX ".pdb";
    auto freetype_pdb_in = std::filesystem::absolute(args.vendor_build / "freetype" / BUILD_MODE / freetype_pdb);
    auto freetype_pdb_out = args.build / BUILD_MODE / freetype_pdb;
    if (std::filesystem::exists(freetype_pdb_in)) {
        std::filesystem::copy_file(freetype_pdb_in, freetype_pdb_out, std::filesystem::copy_options::overwrite_existing);
        ABY_LOG("Copied \"{}\" to \"{}\"", freetype_pdb_in.string(), freetype_pdb_out.string());
    }
    else {
        ABY_ERR("Freetype PDB does not exist: \"{}\"", freetype_pdb_in.string());
        return false;
    }
#endif


    return true;
}

static bool copy_resource_dir(const std::filesystem::path& type, const std::filesystem::path& source_dir, const std::filesystem::path& build_dir) {
    std::filesystem::path resource_dir_in(source_dir / type);
    if (!std::filesystem::exists(resource_dir_in)) {
        ABY_ERR("Resoure directory does not exist: {}", resource_dir_in.string());
        return false;
    }

    std::filesystem::path resource_dir_out(build_dir / BUILD_MODE / type);
    std::filesystem::copy(resource_dir_in, resource_dir_out, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
    ABY_LOG("Copied \"{}\" to \"{}\"", resource_dir_in.string(), resource_dir_out.string());

    return true;
}

static bool copy_resources(const std::filesystem::path& source_dir, const std::filesystem::path& build_dir) {
    if (!copy_resource_dir("Shaders", source_dir, build_dir)) {
        return false;
    }
    if (!copy_resource_dir("Textures", source_dir, build_dir)) {
        return false;
    }
    if (!copy_resource_dir("Fonts", source_dir, build_dir)) {
        return false;
    }
    return true;
}

static void clean_cache(const std::filesystem::path& build_dir) {
    auto cache_dir = build_dir / BUILD_MODE / "Cache";
    if (std::filesystem::exists(cache_dir)) {
        std::filesystem::remove_all(cache_dir);
        ABY_LOG("Removed all files in cache directory: {}", cache_dir.string());
    }
    else {
        ABY_LOG("No Cache existed to be removed");
    }
}

int main(int argc, char** argv) {
    Args args = parse_args(argc, argv);
    return_if(args.ec != EC_SUCCESS);
    return_if(!copy_resources(args.source, args.build));
    return_if(!copy_dlls(args));
    
    if (args.clean) {
        clean_cache(args.build);
    }

    ABY_LOG("Finished!");
    std::cout.flush(); 
    return EC_SUCCESS;
}