#pragma once
#include "Core/Common.h"
#include <imgui/imgui.h>
#include <limits>
namespace aby::imgui {
	
	struct InputConstraints {
		float min	= std::numeric_limits<float>::lowest() / 2;
		float max   = std::numeric_limits<float>::max() / 2;
		float reset = 0.f;
	};

	bool InputFloatEx(const std::string& label, char button, float& v, const InputConstraints& constraints, int components);
	bool InputFloat(const std::string& label, float&  v, const InputConstraints& constraints = {});
	bool InputVec2(const std::string& label,  ImVec2& v, const InputConstraints& constraints = {});
	bool InputVec4(const std::string& label,  ImVec4& v, const InputConstraints& constraints = {});
	bool InputColor(const std::string& label, ImVec4& v, const InputConstraints& constraints = {});


}