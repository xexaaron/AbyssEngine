#include <fstream>
#include <filesystem>
#include <vector>
#include <sstream>
#include <regex>
#include <iostream>

namespace aby {

    static std::vector<std::filesystem::path> get_source_files_r(const std::filesystem::path& root) {
        std::vector<std::filesystem::path> source_files;
        for (const auto& file : std::filesystem::directory_iterator(root)) {
            if (file.is_directory()) {
                auto sub_files = get_source_files_r(file);
                source_files.insert(source_files.end(), sub_files.begin(), sub_files.end());                 
            } else {
                auto ext = file.path().extension();
                if (ext == ".hpp" || ext == ".h" || ext == ".c" || ext == ".cpp" || ext == ".cxx") {
                    source_files.push_back(file.path());
                }
            }
        }
        return source_files;
    }

    static std::vector<std::filesystem::path> get_source_files(const std::filesystem::path& root) {
        auto source_dir = root / "Source";
        if (!std::filesystem::exists(source_dir)) {
            std::cerr << "[Localize] [Error] Project source directory \"" << source_dir.string() << "\" does not exist" << std::endl;
            return {};
        }
    
        std::vector<std::filesystem::path> source_files;
        for (const auto& file : std::filesystem::directory_iterator(source_dir)) {
            if (file.is_directory()) {
                auto sub_files = get_source_files_r(file);
                source_files.insert(source_files.end(), sub_files.begin(), sub_files.end());                 
            } else {
                auto ext = file.path().extension();
                if (ext == ".hpp" || ext == ".h" || ext == ".c" || ext == ".cpp" || ext == ".cxx") {
                    source_files.push_back(file.path());
                }
            }
        }
        return source_files;
    }

    enum class EEncoding {
        U8,
        U16,
        U16W,
        U32,
    };

    void extract_and_write_texts(const std::string& locale, const std::vector<std::filesystem::path>& project_source_files, const std::filesystem::path& output_file) {
        std::regex pattern(R"(Text\((?:u8|u16|u32|u|U|L)?\"(.*?)\"\))");
        std::ofstream ofs(output_file, std::ios::out);
        
        if (!ofs) {
            std::cerr << "[Error] Failed to open output file: " << output_file << std::endl;
            return;
        }

        ofs << "Locale: " << locale << std::endl;

        for (const auto& file : project_source_files) {
            std::ifstream ifs(file);
            if (!ifs.is_open()) {
                std::cerr << "[Warning] Could not open file: " << file << std::endl;
                continue;
            }

            std::stringstream buffer;
            buffer << ifs.rdbuf();
            std::string data = buffer.str();

            std::sregex_iterator it(data.begin(), data.end(), pattern);
            std::sregex_iterator end;

            while (it != end) {
                ofs << it->str(1) << '\n';  // Write extracted text to file
                ++it;
            }
        }

        std::cout << "[Info] Extracted texts written to: " << output_file << std::endl;
    }

    static void print_usage(bool desc = false) {
        std::cerr << "[Localize] [Usage] ./localize <project_root_dir> [OPTIONAL]..." << std::endl;
        std::cerr << "  Options: " << std::endl;
        std::cerr << "   <project_root_dir> : Directory containing ./Source." << std::endl;
        std::cerr << "   --get-text         : Read source files and find text to be localized." << std::endl;
        std::cerr << "   --locale=""        : Specify the locale of strings read from the program." << std::endl;

        if (desc) {
            std::cerr << "  Description:" << std::endl;
            std::cerr << "   1: Get text from the program to be localized in a string database" << std::endl;
        }
    }

}

int main(int argc, char** argv) {
    if (argc < 2) { 
        aby::print_usage(true);
        return 2;
    }
    
    if (!std::filesystem::exists(argv[1])) {
        std::cerr << "[Localize] [Error] Project root directory \"" << argv[1] << "\" does not exist" << std::endl; 
        return 1;
    }
    
    std::vector<std::string> args(argc);
    for (int i = 2; i < argc; i++) { // Start from 1 to skip the program name
        std::string arg = argv[i];
        std::transform(arg.begin(), arg.end(), arg.begin(), [](char c) {
            return static_cast<char>(std::tolower(static_cast<int>(c)));
        });
        args[i - 1] = arg;
    }

    bool get_text = false;
    std::string locale = "EN-US";
    for (auto& arg : args) {
        if (arg == "-h" || arg == "-help" || arg == "--h" || arg == "--help") {
            aby::print_usage(true);
            return 2;
        }
        if (arg == "--get-text") {
            get_text = true;
        }
        if (arg.starts_with("--locale=")) {
            locale = arg.substr(std::size("--locale=") - 1, arg.size() - 1);
        }
    }

    std::filesystem::path project_path(argv[1]);

    if (get_text) {
        std::vector<std::filesystem::path> project_source_files = aby::get_source_files(project_path);
        if (project_source_files.empty()) {
            std::cerr << "[localize] [Error] No source files found" << std::endl;
            return 1;
        }
        aby::extract_and_write_texts(locale, project_source_files, "./Out.txt");
    }

}