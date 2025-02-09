#pragma once
#include "Core/Common.h"
#include "Core/Log.h"
#include <fstream>
#include <span>

namespace aby {

	enum ESerializeMode {
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
		T& read(T& buffer);
		template <typename T> requires (std::is_trivially_constructible_v<T>)
		T& read(T& buffer);
		template <typename T> 
		void write(const T& data);
		template <typename T> requires (std::is_trivially_constructible_v<T>)
		void write(const T& data);
	protected:
		void read_file();
		void create_file();
	private:
		SerializeOpts m_Opts;
		std::int64_t m_Offset;
		std::vector<std::byte> m_Data;
	};


// Do not define this yourself. It is defined inside 'Core/Serialize.cpp'
#ifdef SERIALIZER_IMPL
	template <>
	void Serializer::write<std::string>(const std::string& data) {
		ABY_ASSERT(m_Opts.Mode == ESerializeMode::WRITE, "Cannot write when mode is set to read");
		size_t length = data.size();
		const std::byte* length_bytes = reinterpret_cast<const std::byte*>(&length);
		m_Data.insert(m_Data.end(), length_bytes, length_bytes + sizeof(length));
		const std::byte* string_bytes = reinterpret_cast<const std::byte*>(data.data());
		m_Data.insert(m_Data.end(), string_bytes, string_bytes + data.size());
	}
	
	template <>
	void Serializer::write<const char*>(const char* const& data) {
		ABY_ASSERT(m_Opts.Mode == ESerializeMode::WRITE, "Cannot write when mode is set to read");
		size_t length = std::strlen(data);
		const std::byte* bytes = reinterpret_cast<const std::byte*>(data);
		m_Data.insert(m_Data.end(), bytes, bytes + length);
	}
	template <typename T> requires (std::is_trivially_constructible_v<T>)
	void Serializer::write(const T& data) {
		ABY_ASSERT(m_Opts.Mode == ESerializeMode::WRITE, "Cannot write when mode is set to read");
		auto* bytes = reinterpret_cast<const std::byte*>(&data);
		m_Data.insert(m_Data.end(), bytes, bytes + sizeof(T));
	}

	template <>
	std::string& Serializer::read<std::string>(std::string& buffer) {
		ABY_ASSERT(m_Opts.Mode == ESerializeMode::READ, "Cannot read when mode is set to write");
		size_t length = 0;
		std::memcpy(&length, &m_Data[m_Offset], sizeof(length));
		m_Offset += sizeof(length); 
		buffer.assign(reinterpret_cast<const char*>(&m_Data[m_Offset]), length);
		m_Offset += length; 
		return buffer;
	}

	template <>
	const char*& Serializer::read<const char*>(const char*& buffer) {
		ABY_ASSERT(m_Opts.Mode == ESerializeMode::READ, "Cannot read when mode is set to write");
		size_t length = 0;
		while (m_Offset + length < m_Data.size() && m_Data[m_Offset + length] != std::byte(0)) {
			++length;
		}
		buffer = reinterpret_cast<const char*>(&m_Data[m_Offset]);
		m_Offset += length + 1;
		return buffer;
	}

	template <typename T> requires (std::is_trivially_constructible_v<T>)
	T& Serializer::read(T& buffer) {
		ABY_ASSERT(m_Opts.Mode == ESerializeMode::READ, "Cannot read when mode is set to write");
		ABY_ASSERT(m_Offset + sizeof(T) <= m_Data.size(), "Out of range");
		std::span section(m_Data.begin() + m_Offset, sizeof(T));
		buffer = *reinterpret_cast<T*>(section.data());
		m_Offset += sizeof(T);
		return buffer;
	}
#endif
}