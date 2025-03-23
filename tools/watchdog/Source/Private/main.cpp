/**
* @brief Monitor a program, when that program exits, execute a command. General usage is for restarting a process.
* @param argc 3
* @param argv watchdog <pid or process name> <command>
*/

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include <locale>
#include <codecvt>

#ifdef _WIN32
#include <Windows.h>
#include <tlhelp32.h>
#define popen _popen
#define pclose _pclose
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <limits.h>
#include <dirent.h>
#endif


#define ABY_ERR(fmt, ...) std::cerr << std::format("[Watchdog] [Error] " fmt __VA_OPT__(,) __VA_ARGS__) << '\n';
#define ABY_LOG(fmt, ...) std::cout << std::format("[Watchdog] [Info]  " fmt __VA_OPT__(,) __VA_ARGS__) << '\n';


namespace aby {

#ifdef _WIN32
    auto to_wstring(const std::string& string) -> std::wstring {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, nullptr, 0);
        if (size_needed <= 0) {
            return L"";
        }
        std::wstring wstring(size_needed - 1, L'\0'); // Remove null terminator
        MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, &wstring[0], size_needed);
        return wstring;
    }
#endif

    bool is_process_open(int process_id) {
    #ifdef _WIN32
        // Windows implementation to check if a process is open by ID
        HANDLE proc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, process_id);
        if (proc != NULL) {
            CloseHandle(proc);
            return true;
        }
        return false;
    #else
        // Unix-like systems implementation to check if a process is open by ID
        char buf[PATH_MAX];
        snprintf(buf, sizeof(buf), "/proc/%d", process_id);
        struct stat sts;
        return (stat(buf, &sts) == 0);
    #endif
    }

    bool is_process_open(const std::string& name) {
    #ifdef _WIN32
    // Windows implementation to check if a process is open by name
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            return false;
        }

        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(snapshot, &pe32)) {
            CloseHandle(snapshot);
            return false;
        }

        bool found = false;
        do {
            if (name == pe32.szExeFile) {
                found = true;
                break;
            }
        } while (Process32Next(snapshot, &pe32));

        CloseHandle(snapshot);
        return found;
    #else
    // Unix-like systems implementation to check if a process is open by name
        DIR* dir;
        struct dirent* ent;
        if ((dir = opendir("/proc")) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                // Check if the entry is a directory and represents a numeric process ID
                if (isdigit(*ent->d_name)) {
                    // Read the process cmdline file to get the process name
                    std::string path = std::string("/proc/") + ent->d_name + "/cmdline";
                    std::ifstream file(path);
                    if (file) {
                        std::string cmd_line;
                        std::getline(file, cmd_line);
                        size_t pos = cmd_line.find_last_of('/');
                        std::string found = cmd_line.substr(pos + 1);
                        // Compare process names
                        if (name == found) {
                            closedir(dir);
                            return true;
                        }
                    }
                }
            }
            closedir(dir);
        }
    #endif
        return false;
    }
}
int main(int argc, char* argv[]) {
    if (argc < 3) {
        ABY_ERR("Usage: watchdog <pid/process name> <command>.");
        ABY_ERR(" <pid/process name> If your monitoring a windowed application use the process name not the pid.");
        ABY_ERR(" <command>          The (system) command to execute after the monitored application has shutdown");
        return 1;
    }

    std::string pid_or_name = argv[1];
    std::string command;
    for (int i = 2; i < argc; ++i) {
        command += std::string(argv[i]) + " ";
    }

    bool is_by_name = !isdigit(pid_or_name[0]); // Check if the input is a process name
    int pid = 0;
    if (!is_by_name) {
        pid = std::stoi(pid_or_name);
    }

    while (is_by_name ? aby::is_process_open(pid_or_name) : aby::is_process_open(pid)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::system(command.c_str());

    return 0;
}