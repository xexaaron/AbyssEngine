#include "Utility/File.h"
#include "Platform/Platform.h"

#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif


namespace aby::util {

    FileError fmt_sys_err(const std::filesystem::path& path, const std::string err) {
        return std::format("{}:{}\n\t\t{}", err, path, sys::get_last_err());
    }

}

namespace aby::util {

    MappedFile::MappedFile(const std::filesystem::path& path) :
        m_Data(nullptr),
        m_Size(0),
        m_Error{},
#ifdef _WIN32
        m_Handle(nullptr),
        m_Mapping(nullptr)
#else
        m_FD(-1)
#endif
    {
#ifdef _WIN32
        m_Handle = CreateFileA(
            path.string().c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr
        );


        if (m_Handle == INVALID_HANDLE_VALUE) {
            m_Error = fmt_sys_err(path, "Failed to create file handle");
            return;
        }

        LARGE_INTEGER size;
        if (!GetFileSizeEx(m_Handle, &size)) {
            m_Error = fmt_sys_err(path, "Failed to get file size");
            CloseHandle(m_Handle);
            m_Handle = nullptr;
            return;
        }

        m_Size = static_cast<size_t>(size.QuadPart);

        m_Mapping = CreateFileMappingA(
            m_Handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!m_Mapping) {
            m_Error = fmt_sys_err(path, "Failed to create file mapping"); 
            CloseHandle(m_Handle);
            m_Handle = nullptr;
            return;
        }

        m_Data = MapViewOfFile(m_Mapping, FILE_MAP_READ, 0, 0, 0);
        if (!m_Data) {
            m_Error = fmt_sys_err(path, "Failed to map file");
            CloseHandle(m_Mapping);
            CloseHandle(m_Handle);
            m_Mapping = nullptr;
            m_Handle = nullptr;
            return;
        }

#else
        m_FD = open(path.c_str(), O_RDONLY);
        if (m_FD == -1) {
            m_Error = std::format("Failed to open file: {}\n\t\t{}", path, strerror(errno));
            return;
        }

        struct stat st;
        if (fstat(m_FD, &st) == -1) {
            m_Error = std::format("Failed to get file stats: {}\n\t\t{}", path, strerror(errno));
            close(m_FD);
            m_FD = -1;
            return;
        }

        m_Size = static_cast<size_t>(st.st_size);
        m_Data = mmap(nullptr, m_Size, PROT_READ, MAP_PRIVATE, m_FD, 0);
        if (m_Data == MAP_FAILED) {
            m_Error = std::format("Failed to map file into memory: {}\n\t\t{}", path, strerror(errno));
            close(m_FD);
            m_Data = nullptr;
            m_FD = -1;
            return;
        }
#endif
    }
    
    MappedFile::~MappedFile() {
#ifdef _WIN32
        if (m_Data) UnmapViewOfFile(m_Data);
        if (m_Mapping) CloseHandle(m_Mapping);
        if (m_Handle) CloseHandle(m_Handle);
#else
        if (m_Data) munmap(m_Data, m_Size);
        if (m_FD != -1) close(m_FD);
#endif
    }

    std::string_view MappedFile::view() const {
        return { static_cast<const char*>(m_Data), m_Size };
    }

    FileError MappedFile::error() const {
        return m_Error;
    }

    std::size_t MappedFile::size() const {
        return m_Size;
    }


    MappedFile::operator bool() const {
        return m_Data != nullptr;
    }

}

namespace aby::util {

    File::File(const std::filesystem::path& path) : 
        m_Path(path)
    {

    }

    std::expected<std::string, FileError> File::read(EFileMode mode) {
        std::ifstream ifs(m_Path, std::ios::in | static_cast<std::ios::openmode>(mode));

        if (!ifs.is_open())
            return std::unexpected(std::format("Failed to open file for reading: {}", m_Path));

        std::stringstream ss;
        ss << ifs.rdbuf();  

        return ss.str();
    }

    FileError File::write(const std::string& data, EFileMode mode) {
        std::ofstream ofs(m_Path, static_cast<std::ios::openmode>(mode));

        if (!ofs.is_open())
            return std::format("Failed to open file for writing: {}", m_Path);

        ofs << data;
        if (!ofs)  
            return std::format("Failed to write to file: {}", m_Path);

        return {};
    }

    std::expected<MappedFile, FileError> File::map() {
        MappedFile file(m_Path);
        if (!file) return std::unexpected(file.error());
        return std::move(file);
    }

    bool File::exists() const {
        return std::filesystem::exists(m_Path);
    }
    
    File::operator bool() const {
        return this->exists();
    } 

}