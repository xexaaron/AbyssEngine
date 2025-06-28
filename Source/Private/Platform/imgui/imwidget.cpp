#include "Platform/imgui/imwidget.h"

namespace aby::imgui {

	bool InputFloatEx(const std::string& label, char button, float& v, const InputConstraints& constraints, int components) {
		ABY_ASSERT(constraints.reset >= constraints.min && constraints.reset <= constraints.max, "Reset value is invalid");
		ABY_ASSERT(components > 0, "Invalid amount of components");
		
		const float button_width = 22.5f;
		const float avail = ImGui::GetContentRegionAvail().x;
		const float slider_width = (avail - (button_width)*components) / components;
		bool reset = false;
		bool result = false;
		bool pushed = false;
		std::string slider_id = "##" + label + "_" + button;
		std::string button_id = std::string(1, button) + slider_id;
		
		switch (button) {
			case 'x': case 'r': ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.f, 0.f, 0.f, 1.f)); pushed = true; break;
			case 'y': case 'g': ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 1.f, 0.f, 1.f)); pushed = true; break;
			case 'z': case 'b': ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 1.f, 1.f)); pushed = true; break;
			case 'w': case 'a': ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.f)); pushed = true; break;
		}

		ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0);

		reset = ImGui::Button(button_id.c_str(), ImVec2(button_width, 22.5f));
		
		if (pushed) ImGui::PopStyleColor();

		ImGui::SameLine();
		ImGui::SetNextItemWidth(slider_width);
		
		result = ImGui::SliderFloat(slider_id.c_str(), &v, constraints.min, constraints.max);

		if (reset) v = constraints.reset;

		ImGui::PopStyleVar();

		return result;
	}

	bool InputFloat(const std::string& label, float& v, const InputConstraints& constraints) {
		return InputFloatEx(label, 'x', v, constraints, 1);
	}
	
	bool InputVec2(const std::string& label, ImVec2& v, const InputConstraints& constraints) {
		bool result = false;

		ImGui::PushID(label.c_str());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, 0);

		result |= InputFloatEx(label, 'x', v.x, constraints, 2);
		ImGui::SameLine();
		result |= InputFloatEx(label, 'y', v.y, constraints, 2);

		ImGui::PopStyleVar();
		ImGui::PopID();

		return result;
	}

	bool InputVec4(const std::string& label, ImVec4& v, const InputConstraints& constraints) {
		bool result = false;

		ImGui::PushID(label.c_str());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, 0);

		result |= InputFloatEx(label, 'x', v.x, constraints, 3);
		ImGui::SameLine();
		result |= InputFloatEx(label, 'y', v.y, constraints, 3);
		ImGui::SameLine();
		result |= InputFloatEx(label, 'z', v.z, constraints, 3);
		ImGui::SameLine();
		result |= InputFloatEx(label, 'w', v.w, constraints, 3);

		ImGui::PopStyleVar();
		ImGui::PopID();

		return result;
	}

	bool InputColor(const std::string& label, ImVec4& v, const InputConstraints& constraints) {
		bool result = false;

		ImGui::PushID(label.c_str());
		ImGui::PushStyleVarX(ImGuiStyleVar_ItemSpacing, 0);

		result |= InputFloatEx(label, 'r', v.x, constraints, 4);
		ImGui::SameLine();
		result |= InputFloatEx(label, 'g', v.y, constraints, 4);
		ImGui::SameLine();
		result |= InputFloatEx(label, 'b', v.z, constraints, 4);
		ImGui::SameLine();
		result |= InputFloatEx(label, 'a', v.w, constraints, 4);

		ImGui::PopStyleVar();
		ImGui::PopID();

		return result;
	}
}
