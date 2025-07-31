#include "Utility/Serialize.h"
#include "Utility/File.h"
#include <fstream>

namespace aby::util {
    Serializer::Serializer(const SerializeOpts& opts) : m_Opts(opts), m_Offset(0) {
        set_mode(opts.mode);
    }
    
    void Serializer::reset() {
        m_Data.clear();
        m_Offset = 0;
    }
    void Serializer::seek(i64 offset) {
        m_Offset += offset;
        ABY_ASSERT(m_Offset >= 0, "Out of range");
        ABY_ASSERT(m_Offset < static_cast<int64_t>(m_Data.size()), "Out of range");
    }
    void Serializer::set_mode(ESerializeMode mode) {
        m_Opts.mode = mode;
        if (mode == ESerializeMode::READ) {
            reset();
            read_file();
        }
        else if (mode == ESerializeMode::WRITE) {
            create_file();
        }
    }
    void Serializer::save() {
        if (m_Data.empty()) {
            ABY_WARN("Attempting to save serialized data but Serializer::m_Data is empty");
            return;
        }
        std::ofstream ofs(m_Opts.file, std::ios::binary);
        if (ofs.is_open()) {
            ofs.write(reinterpret_cast<const char*>(m_Data.data()), m_Data.size());
            ofs.close();
        }
        else {
            ABY_ERR("Failed to open file for writing: {}", m_Opts.file);
        }
    }

    void Serializer::read_file() {
        util::MappedFile file(m_Opts.file);
        if (!file) {
            ABY_ERR("{}", file.error());
            return;
        }
        m_Data.resize(file.size());
        std::memcpy(m_Data.data(), file.view().data(), file.size());
    }
    
    void Serializer::create_file() {
        if (!std::filesystem::exists(m_Opts.file.parent_path())) {
            std::filesystem::create_directories(m_Opts.file.parent_path());
        }
        std::ofstream ofs(m_Opts.file, std::ios::binary | std::ios::trunc);
        if (ofs.is_open()) {
            ofs.close();
        }
        else {
            ABY_ERR("Failed to open file for writing: {}", m_Opts.file);
        }
    }

}