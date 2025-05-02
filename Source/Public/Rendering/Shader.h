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
		std::span<const u32> data() const;
	protected:
		Shader(const std::vector<u32>& data, EShader type);
	protected:
		EShader m_Type = EShader::MAX_ENUM;
		std::vector<u32> m_Data;
	};
}

namespace std {
	std::string to_string(aby::EShader type);
}