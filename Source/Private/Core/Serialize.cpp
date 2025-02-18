#include "Core/Serialize.h"

namespace aby {
    Serializer::Serializer(const SerializeOpts& opts) : m_Opts(opts), m_Offset(0) {
        set_mode(opts.Mode);
    }
    
    void Serializer::reset() {
        m_Data.clear();
        m_Offset = 0;
    }
    void Serializer::seek(std::int64_t offset) {
        m_Offset += offset;
        ABY_ASSERT(m_Offset >= 0, "Out of range");
        ABY_ASSERT(m_Offset < m_Data.size(), "Out of range");
    }
    void Serializer::set_mode(ESerializeMode mode) {
        m_Opts.Mode = mode;
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
        std::ofstream ofs(m_Opts.File, std::ios::binary);
        if (ofs.is_open()) {
            ofs.write(reinterpret_cast<const char*>(m_Data.data()), m_Data.size());
            ofs.close();
        }
        else {
            ABY_ERR("Failed to open file for writing: {}", m_Opts.File.string());
        }
    }

    void Serializer::read_file() {
        std::ifstream ifs(m_Opts.File, std::ios::binary);
        if (ifs.is_open()) {
            ifs.seekg(0, std::ios::end);
            std::streamsize size = ifs.tellg();
            ifs.seekg(0, std::ios::beg);
            m_Data.resize(size);
            ifs.read(reinterpret_cast<char*>(m_Data.data()), size);
            ifs.close();
        }
        else {
            ABY_ERR("Failed to open file for reading: {}", m_Opts.File.string());
        }
    }
    
    void Serializer::create_file() {
        if (!std::filesystem::exists(m_Opts.File.parent_path())) {
            std::filesystem::create_directories(m_Opts.File.parent_path());
        }
        std::ofstream ofs(m_Opts.File, std::ios::binary | std::ios::trunc);
        if (ofs.is_open()) {
            ofs.close();
        }
        else {
            ABY_ERR("Failed to open file for writing: {}", m_Opts.File.string());
        }
    }

}