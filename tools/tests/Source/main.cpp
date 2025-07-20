#include "Framework.h"
#include <Utility/File.h>
#include <Platform/Platform.h>
#include <filesystem>
#include <fstream>

TEST(File) {
    fs::path path = "./Temp.Text";
    std::ofstream ofs(path, std::ios::trunc);
    std::string lorem_ipsum =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do"
        "\neiusmod tempor incididunt ut labore et dolore magna aliqua. Ut"
        "\nenim ad minim veniam, quis nostrud exercitation ullamco laboris"
        "\nnisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in"
        "\nreprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
        "\npariatur. Excepteur sint occaecat cupidatat non proident, sunt in"
        "\nculpa qui officia deserunt mollit anim id est laborum.";

    if (ofs.is_open()) {
        ofs << lorem_ipsum;
        ofs.close();
    }

    aby::util::File file(path);
    if (!file) {
        File::err("File does not exist");
        return false;
    }

    auto data = file.read();
    if (!data) {
        File::err("{}", data.error());
        return false;
    }

    std::string value = data.value();
    if (value != lorem_ipsum) {
        File::err("File data incorrect. File Data: {}", value);
        return false;
    }

    fs::remove(path);
    return true;
}

TEST(MappedFile) {
    fs::path path = "./Temp.Text";
    std::ofstream ofs(path, std::ios::trunc | std::ios::binary);
    std::string lorem_ipsum =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do"
        "\neiusmod tempor incididunt ut labore et dolore magna aliqua. Ut"
        "\nenim ad minim veniam, quis nostrud exercitation ullamco laboris"
        "\nnisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in"
        "\nreprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
        "\npariatur. Excepteur sint occaecat cupidatat non proident, sunt in"
        "\nculpa qui officia deserunt mollit anim id est laborum.";

    if (ofs.is_open()) {
        ofs << lorem_ipsum;
        ofs.close();
    }
    
    {
        aby::util::MappedFile file(path);
        if (!file) {
            MappedFile::err("File data invalid");
            return false;
        }

        if (!file.error().empty()) {
            std::string err = file.error().c_str();
            MappedFile::err("{}", err);
        }


        std::string_view value = file.view();
        if (std::string(value) != lorem_ipsum) {
            auto data = std::string(value);
            MappedFile::err("File data incorrect.", data);
            return false;
        }
    }

    fs::remove(path);
    return true;
}

int main() {
    if (!aby::TestFramework::get().run()) {
        return 1;
    }
    return 0;
}