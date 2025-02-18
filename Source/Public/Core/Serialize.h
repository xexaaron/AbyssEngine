#pragma once
#include "Core/Common.h"
#include "Core/Log.h"
#include <fstream>
#include <span>

namespace aby {

	enum class ESerializeMode {
		READ,
		WRITE,
	};

	struct SerializeOpts {
		fs::path  	   File;
		ESerializeMode Mode;
	};

	class Serializer {
	public:
		Serializer(const SerializeOpts& opts);

		void save();
		void reset();
		void seek(std::int64_t offset);

		void set_mode(ESerializeMode mode);

		template <typename T>
		void write(const T& data) {
			ABY_ASSERT(m_Opts.Mode == ESerializeMode::WRITE, "Cannot write when mode is set to read");
			if constexpr (std::is_same_v<T, std::string>) {
				size_t length = data.size();
				const std::byte* length_bytes = reinterpret_cast<const std::byte*>(&length);
				m_Data.insert(m_Data.end(), length_bytes, length_bytes + sizeof(length));
				const std::byte* string_bytes = reinterpret_cast<const std::byte*>(data.data());
				m_Data.insert(m_Data.end(), string_bytes, string_bytes + length);
			}
			else if constexpr (std::is_same_v<T, const char*>) {
				size_t length = std::strlen(data);
				const std::byte* bytes = reinterpret_cast<const std::byte*>(data);
				m_Data.insert(m_Data.end(), bytes, bytes + length);
			}
			else if constexpr (std::is_trivially_constructible_v<T>) {
				const std::byte* bytes = reinterpret_cast<const std::byte*>(&data);
				m_Data.insert(m_Data.end(), bytes, bytes + sizeof(T));
			}
		}

		template <typename T>
		T& read(T& buffer) {
			ABY_ASSERT(m_Opts.Mode == ESerializeMode::READ, "Cannot read when mode is set to write");
			if constexpr (std::is_same_v<T, std::string>) {
				size_t length = 0;
				std::memcpy(&length, &m_Data[m_Offset], sizeof(length));
				m_Offset += sizeof(length);
				buffer.assign(reinterpret_cast<const char*>(&m_Data[m_Offset]), length);
				m_Offset += length;
			}
			else if constexpr (std::is_same_v<T, const char*>) {
				size_t length = 0;
				while (m_Offset + length < m_Data.size() && m_Data[m_Offset + length] != std::byte(0)) {
					++length;
				}
				buffer = reinterpret_cast<const char*>(&m_Data[m_Offset]);
				m_Offset += length + 1;
			}
			else if constexpr (std::is_trivially_constructible_v<T>) {
				ABY_ASSERT(m_Offset + sizeof(T) <= m_Data.size(), "Out of range");
				std::memcpy(&buffer, &m_Data[m_Offset], sizeof(T));
				m_Offset += sizeof(T);
			}
			return buffer;
		}

	protected:
		void read_file();
		void create_file();
	private:
		SerializeOpts m_Opts;
		std::int64_t m_Offset;
		std::vector<std::byte> m_Data;
	};

	
}