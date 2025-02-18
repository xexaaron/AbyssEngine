#include "Rendering/Shader.h"
#include "vk/VkShader.h"
#include "vk/VkContext.h"
#include "Core/Log.h"

namespace aby {
	
	namespace helper {
		std::string EShader_to_string(EShader type) {
			switch (type) {
				case EShader::VERTEX:
					return "Vertex";
				case EShader::FRAGMENT:
					return "Fragment";
				default:
					ABY_ASSERT(false, "EShader out of bounds");
					break;
			}
			return "UNREACHABLE";
		}
	}

	Shader::Shader(const std::vector<std::uint32_t>& data, EShader type) : 
		m_Type(type), m_Data(data)
	{
		
	}

	Resource Shader::create(Context* ctx, const fs::path& path, EShader type) {
		switch (ctx->backend()) {
			case EBackend::VULKAN: {
				auto shader = vk::Shader::create(
					static_cast<vk::Context*>(ctx)->devices(),
					path,
					type
				);
				return ctx->shaders().add(shader);
			}
			default:
				ABY_ASSERT(false, "Ctx backend is invalid");
		}
		return {};
	}

	EShader Shader::type() const {
		return m_Type;
	}

	std::span<const std::uint32_t> Shader::data() const {
		return std::span(m_Data.begin(), m_Data.size());
	}

}