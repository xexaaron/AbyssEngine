#pragma once

#include "Core/Common.h"
#include "Core/Resource.h"
#include <span>

namespace aby {
	
	enum class EShader {
		FROM_EXT = -1, // ie .frag, .fragment, .vert, .vertex
		VERTEX   = 0,
		FRAGMENT = 1,
		MAX_ENUM,
	};

	class Context;

	class Shader {
	public:
		static Resource create(Context* ctx, const fs::path& path, EShader type = EShader::FROM_EXT);

		EShader type() const;
		std::span<const std::uint32_t> data() const;
	protected:
		Shader(const std::vector<std::uint32_t>& data, EShader type);
	protected:
		EShader m_Type = EShader::MAX_ENUM;
		std::vector<std::uint32_t> m_Data;
	};
}

namespace std {
	std::string to_string(aby::EShader type);
}