#include "Core/Common.h"
#include "Core/Log.h"
#include <expected>

namespace aby::util {

    enum class EFileMode {
        NONE    =      0,
        ATE     =  BIT(2),
        APP     =  BIT(3),
        TRUNC   =  BIT(4),
        BINARY  =  BIT(5),
    };
    DECLARE_ENUM_OPS(EFileMode);

    /**
    * Empty on success
    */
    using FileError = std::string;

    class MappedFile {
    public:
        MappedFile(const std::filesystem::path& path);
        ~MappedFile();
        MappedFile(const MappedFile&) = delete;
        MappedFile(MappedFile&&) noexcept = default;

        std::string_view view() const;
        FileError error() const;
        std::size_t size() const;

        explicit operator bool() const;
        MappedFile& operator=(const MappedFile&) = delete;
        MappedFile& operator=(MappedFile&&) noexcept = default;
    private:
        void* m_Data;
        size_t m_Size;
        FileError m_Error;
#ifdef _WIN32
        void* m_Handle;
        void* m_Mapping;
#else
        int m_FD;
#endif
    };

    class File {
    public:
        File(const std::filesystem::path& path);

        std::expected<std::string, FileError> read(EFileMode mode = EFileMode::NONE);
        std::expected<MappedFile, FileError>  map();

        FileError write(const std::string& data, EFileMode mode = EFileMode::NONE);

        bool exists() const;
        
        /// Exists Operator
        explicit operator bool() const; 
    private:
        std::filesystem::path m_Path;
    };

}