#ifdef _WIN32

#include "Platform/win32/ProcessWin32.h"
#include "Core/Log.h"
#include "Core/App.h"
#include <istream>
#include <regex>
#include <array>

namespace aby::sys::win32 {

    std::string get_last_err() {
        DWORD errorMessageID = GetLastError();
        if (errorMessageID == 0) {
            return std::string(); // No error message has been recorded
        }

        LPSTR messageBuffer = nullptr;
        size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, nullptr);

        std::string message(messageBuffer, size);

        // Free the buffer allocated by FormatMessage()
        LocalFree(messageBuffer);

        return message;
    }

	Process::Process(const std::function<void(const std::string&)>& read_callback) :
		sys::Process(read_callback),
        bRunning(false),
        m_Thread(nullptr)
	{
		
	}

	Process::~Process() {
		close();
	}

    bool Process::open(const std::string& cmd) {
        create_console();
        std::string           cmdline         = cmd;
        STARTUPINFOEXA        si              = create_startup_info();
        SECURITY_ATTRIBUTES   sa{ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        BOOL                  inherit_handles = TRUE;
        DWORD                 flags           = EXTENDED_STARTUPINFO_PRESENT;
        LPVOID                env             = NULL;
        LPSTR                 cwd             = NULL;
        PROCESS_INFORMATION   pi              = {};
        ZeroMemory(&pi, sizeof(pi));
        if (!CreateProcessA(
            NULL,
            cmdline.data(),
            &sa,
            &sa,
            inherit_handles,
            flags,
            env,
            cwd,
            &si.StartupInfo,
            &pi))
        {
            ABY_ERR("Unknown command: {}", cmdline);
            CloseHandle(m_Handles.in.read);  
            CloseHandle(m_Handles.out.write);
            return false;
        }
        CloseHandle(m_Handles.in.read);   // Close parent's stdin read-end
        CloseHandle(m_Handles.out.write); // Close parent's stdout write-end
        
        bRunning.store(true);
        m_Thread  = create_unique<Thread>([this]() { return this->loop(); }, split(cmd)[0]);
        m_Handles.proc = pi.hProcess;
        return true;
    }

	void Process::close() {
        if (is_open()) {
            bRunning.store(false);
            
            m_Thread->join();
            m_Thread.reset();

            ClosePseudoConsole(m_Handles.con);
            CloseHandle(m_Handles.in.write);
            CloseHandle(m_Handles.out.read);
        }
	}

    void Process::loop() {
        char buffer[1024];
        while (bRunning.load() == true) {
            DWORD ec;   
            if (GetExitCodeProcess(m_Handles.proc, &ec) && ec != STILL_ACTIVE) {
                ABY_LOG("Process exited with code: {}", ec);
                bRunning.store(false);
                break;
            }
            while (!m_Writes.empty()) {
                std::string input = m_Writes.front();
                m_Writes.pop();
                DWORD bytes_written;
                WriteFile(m_Handles.in.write, input.c_str(), static_cast<DWORD>(input.size()), &bytes_written, NULL);
            }
            DWORD bytes_avail = 0;
            if (PeekNamedPipe(m_Handles.out.read, NULL, 0, NULL, &bytes_avail, NULL)) {
                if (bytes_avail > 0) {
                    DWORD bytes_read;
                    if (ReadFile(m_Handles.out.read, buffer, sizeof(buffer) - 1, &bytes_read, NULL)) {
                        buffer[bytes_read] = '\0';
                        std::string str(buffer);
                        sanitize(str);
                        std::istringstream stream(str);
                        std::string line;
                        while (std::getline(stream, line)) {
                            m_Read(line);
                        }
                    } else {
                        DWORD error = GetLastError();
                        if (error == ERROR_BROKEN_PIPE || error == ERROR_NO_DATA) {
                            break;  // Exit loop if no more data
                        }
                        else {
                            ABY_LOG("Failed to read process stdin: {}", get_last_err());  // Log any other errors
                        }
                    }
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));  // Avoid busy-waiting
        }
    }

    void Process::write(const std::string& data) {
        m_Writes.push(data + "\r\n");  
    }
    
    bool Process::is_open() {
        return bRunning.load();
    }

    void Process::create_console() {
        SECURITY_ATTRIBUTES sa{ sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

        if (!CreatePipe(&m_Handles.in.read, &m_Handles.in.write, &sa, 0))
        {
            WIN32_CHECK(HRESULT_FROM_WIN32(GetLastError()));
        }

        if (!CreatePipe(&m_Handles.out.read, &m_Handles.out.write, &sa, 0))
        {
            WIN32_CHECK(HRESULT_FROM_WIN32(GetLastError()));
        }

        COORD size  = { .X = 80, .Y = 25 };
        DWORD flags = 0;
        WIN32_CHECK(CreatePseudoConsole(size, m_Handles.in.read, m_Handles.out.write, flags, &m_Handles.con));
    }

    STARTUPINFOEX Process::create_startup_info() {
        STARTUPINFOEXA si;
        ZeroMemory(&si, sizeof(si));
        si.StartupInfo.cb = sizeof(STARTUPINFOEXA);

        // Discover the size required for the list
        size_t required_bytes = 0;
        InitializeProcThreadAttributeList(NULL, 1, 0, &required_bytes);

        // Allocate memory to represent the list
        si.lpAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, required_bytes);
        if (!si.lpAttributeList)
        {
            throw std::out_of_range("Out of memory");
        }

        // Initialize the list memory location
        if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &required_bytes))
        {
            HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
            WIN32_CHECK(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Set the pseudoconsole information into the list
        if (!UpdateProcThreadAttribute(si.lpAttributeList,
            0,
            PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
            m_Handles.con,
            sizeof(m_Handles.con),
            NULL,
            NULL))
        {
            HeapFree(GetProcessHeap(), 0, si.lpAttributeList);
            WIN32_CHECK(HRESULT_FROM_WIN32(GetLastError()));
        }

        return si;
    }

    std::string replace_cursor_right_with_spaces(const std::string& input) {
    // This regex matches the ANSI escape sequence for moving the cursor right.
    // It captures one or more digits before the 'C'.
        std::regex pattern("\x1B\\[([0-9]+)C");
        std::string result;

        // We'll use regex iterators to process each match.
        std::sregex_iterator it(input.begin(), input.end(), pattern);
        std::sregex_iterator end;

        // Track the position in the input string.
        size_t lastPos = 0;

        for (; it != end; ++it) {
            std::smatch match = *it;
            // Append text from the last position up to the match.
            result.append(input.substr(lastPos, match.position() - lastPos));

            // Convert the captured digits to an integer.
            int nSpaces = std::stoi(match[1].str());
            // Append nSpaces spaces.
            result.append(std::string(nSpaces, ' '));

            // Update lastPos to be the end of the current match.
            lastPos = match.position() + match.length();
        }

        // Append any remaining text after the last match.
        result.append(input.substr(lastPos));

        return result;
    }

    void Process::sanitize(std::string& buffer) {
        // Remove ANSI escape codes
        std::regex ansi_regex("\x1B\\[[0-9;]*[mG]");
        // Remove common Windows terminal control sequences
        std::regex win_regex("\x1B\\[(2J|H|25h)");

        buffer = std::regex_replace(buffer, ansi_regex, "");
        buffer = std::regex_replace(buffer, win_regex, "");

        std::string sanitized;
        sanitized.reserve(buffer.size() * 2); 
        for (char ch : buffer) {
            if (ch == '\\') {
                // Convert Windows-style paths to Unix-style
                sanitized += "/"; 
            }
            else if (ch == '\t') {
                sanitized += "    ";
            }
            else {
                sanitized += ch;
            }
        }

        // Remove beginning of msg esc sequence.
        constexpr char esc = '\x1b';
        constexpr std::array begin_of_msg = { esc, '[', '?', '2', '5', '1' };
        if (sanitized[0]) {
            std::size_t i = 0;
            for (; i < begin_of_msg.size() && sanitized[i] == begin_of_msg[i]; ++i) {}
            if (i == begin_of_msg.size() - 1) {
                sanitized = sanitized.substr(begin_of_msg.size());
            }
        }
        // Remove ']0;FILE' from end of msg
        constexpr std::array end_of_msg = { esc, ']', '0', ';', };
        if (sanitized.length() >= end_of_msg.size()) {
            for (std::size_t i = 0; i < sanitized.size() - end_of_msg.size() + 1; i++) {
                if (sanitized[i + 0] == end_of_msg[0] &&
                    sanitized[i + 1] == end_of_msg[1] &&
                    sanitized[i + 2] == end_of_msg[2] &&
                    sanitized[i + 3] == end_of_msg[3])
                {
                    sanitized.erase(i);
                    break;
                }
            }
        }
        buffer = replace_cursor_right_with_spaces(sanitized);
    }

    void sanitize_cursor(std::string& buffer) {
        constexpr const char* ESC = "\x1B[";
        std::size_t pos = 0;

        while ((pos = buffer.find(ESC, pos)) != std::string::npos) {
            std::size_t begin_seq = pos + std::strlen(ESC);
            std::size_t cursor = 0;
            std::string n, x, y;
            char cmd = '\0';

            // Safeguard for buffer overrun
            if (begin_seq >= buffer.size()) break;

            // Collect digits for n
            while ((begin_seq + cursor) < buffer.size() && std::isdigit(buffer[begin_seq + cursor])) {
                n += buffer[begin_seq + cursor++];
            }

            // Check for ; indicating y;x
            if ((begin_seq + cursor) < buffer.size() && buffer[begin_seq + cursor] == ';') {
                y = n;
                n.clear();
                cursor++; // skip ';'

                // Collect digits for x
                while ((begin_seq + cursor) < buffer.size() && std::isdigit(buffer[begin_seq + cursor])) {
                    x += buffer[begin_seq + cursor++];
                }
            }

            // Now fetch the command character
            if ((begin_seq + cursor) < buffer.size() && std::isalpha(buffer[begin_seq + cursor])) {
                cmd = buffer[begin_seq + cursor++];
            }
            else {
                // Invalid or incomplete sequence
                pos += 1;
                continue;
            }
            // Calculate total length of the escape sequence
            std::size_t total_len = cursor + std::strlen(ESC);

            // Handle ESC[<n>C – Cursor Forward (right)
            if (cmd == 'C' && !n.empty()) {
                int num_spaces = std::stoi(n);
                buffer.replace(pos, total_len, std::string(num_spaces, ' '));
                pos += num_spaces; // Advance past the inserted spaces
            }
            else {
                // Erase any other ESC sequence
                buffer.erase(pos, total_len);
            }
        }
    }


    std::vector<std::string> Process::split(const std::string& cmd) {
        std::vector<std::string> out;
        std::istringstream stream(cmd);
        std::string token;
        while (stream >> token) {
            out.push_back(token);
        }
        return out;
    }

    void Process::kill() {
        this->close();
        ABY_ERR("Process recieved signal: SIGINT");
    }

}

#endif