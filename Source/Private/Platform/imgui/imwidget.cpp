#include "Platform/imgui/imwidget.h"
#include "Utility/TagParser.h"
#include <imgui/imgui_internal.h>

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

	void UnderlinePreviousText(ImGuiCol col) {
		ImVec2 text_pos = ImGui::GetItemRectMin();
		ImVec2 text_size = ImGui::GetItemRectSize();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec4 color = ImGui::GetStyle().Colors[ImGuiCol_TextDisabled];
		ImU32 packed_color = ImGui::ColorConvertFloat4ToU32(color);
		draw_list->AddLine(
			ImVec2(text_pos.x, text_pos.y + text_size.y),
			ImVec2(text_pos.x + text_size.x, text_pos.y + text_size.y),
			packed_color,
			1.0f
		);
	}

	void TextWithTags(const std::string& text, bool wrapped) {
		std::string ctext = text;
		auto decors = util::parse_and_strip_tags(ctext);
		int pos = 0;
		bool first = true;

		for (const auto& decor : decors) {
			// Normal (unstyled) text before tag
			if (pos < decor.range.start) {
				std::string segment = ctext.substr(pos, decor.range.start - pos);
				if (!first) ImGui::SameLine(0.f, 0.f);
				ImGui::TextUnformatted(segment.c_str());
				first = false;
			}

			// Tagged segment
			std::string segment = ctext.substr(decor.range.start, decor.range.end - decor.range.start + 1);

			if (!first) ImGui::SameLine(0.f, 0.f);

			switch (decor.type) {
				case util::ETextDecor::FILE_PATH: {
					fs::path path(segment);
					std::string name = "\"" + path.filename().string();
					TextLink(name.c_str(), segment.c_str());
				} break;
				case util::ETextDecor::URI_LINK:
					TextLink(segment.c_str());
					break;
				default:
					throw std::runtime_error("Unsupported text tag");
			}
			first = false;
			pos = static_cast<int>(decor.range.end) + 1;
		}

		// Remaining plain text
		if (pos < ctext.size()) {
			std::string segment = ctext.substr(pos);
			if (!first) ImGui::SameLine(0.f, 0.f);
			ImGui::TextUnformatted(segment.c_str());
		}
	}

	void TextLink(const std::string& text, std::string url) {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		const char* label = text.c_str();
		const ImGuiID id = window->GetID(label);
		const char* label_end = ImGui::FindRenderedTextEnd(label);
		ImVec2 pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrLineTextBaseOffset);
		ImVec2 size = ImGui::CalcTextSize(label, label_end, true);
		ImRect bb(pos, pos + size);
		bool hovered, held;
		ImVec4 text_colf = g.Style.Colors[ImGuiCol_TextLink];
		ImVec4 line_colf = text_colf;

		if (url.empty())
			url = text;
		
		ImGui::ItemSize(size, 0.0f);
		if (!ImGui::ItemAdd(bb, id))
			return;

		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

		ImGui::RenderNavCursor(bb, id);

		if (hovered)
			ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

		{
			float h, s, v;
			ImGui::ColorConvertRGBtoHSV(text_colf.x, text_colf.y, text_colf.z, h, s, v);
			if (held || hovered)
			{
				v = ImSaturate(v + (held ? 0.4f : 0.3f));
				h = ImFmod(h + 0.02f, 1.0f);
			}
			ImGui::ColorConvertHSVtoRGB(h, s, v, text_colf.x, text_colf.y, text_colf.z);
			v = ImSaturate(v - 0.20f);
			ImGui::ColorConvertHSVtoRGB(h, s, v, line_colf.x, line_colf.y, line_colf.z);
		}
		float line_y = bb.Max.y + ImFloor(g.Font->LastBaked->Descent * g.FontBakedScale * 0.20f);
		window->DrawList->AddLine(ImVec2(bb.Min.x, line_y), ImVec2(bb.Max.x, line_y), ImGui::GetColorU32(line_colf)); // FIXME-TEXT: Underline mode // FIXME-DPI

		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(text_colf));
		ImGui::RenderText(bb.Min, label, label_end);
		ImGui::PopStyleColor();

		if (pressed && ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
			if (g.PlatformIO.Platform_OpenInShellFn != NULL)
				g.PlatformIO.Platform_OpenInShellFn(&g, url.c_str());
		}
	}

	bool ImageTreeNode(const void* id, const std::string& label, ImTextureID img, ImVec2 icon_size, ImGuiTreeNodeFlags flags)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		// Generate ID
		ImGuiID node_id = window->GetID(id);

		// Figure out row size
		ImVec2 label_size = ImGui::CalcTextSize(label.c_str());
		float row_height = ImMax(icon_size.y, label_size.y) + style.FramePadding.y * 2.0f;
		ImVec2 pos = ImGui::GetCursorScreenPos();

		// Setup bounding box for the whole row
		ImRect total_bb(pos, ImVec2(pos.x + window->WorkRect.Max.x, pos.y + row_height));
		ImGui::ItemSize(total_bb);
		if (!ImGui::ItemAdd(total_bb, node_id))
			return false;

		// Check open/close state
		bool is_open = ImGui::TreeNodeGetOpen(node_id);

		// Arrow
		if ((flags & ImGuiTreeNodeFlags_Leaf) == 0) {
			ImGui::RenderArrow(window->DrawList,
				ImVec2(pos.x + style.FramePadding.x, pos.y + style.FramePadding.y),
				ImGui::GetColorU32(ImGuiCol_Text),
				is_open ? ImGuiDir_Down : ImGuiDir_Right);
		}

		// Cursor after arrow
		float arrow_offset = (flags & ImGuiTreeNodeFlags_Leaf) ? 0.0f : ImGui::GetTreeNodeToLabelSpacing();
		ImGui::SetCursorScreenPos(ImVec2(pos.x + arrow_offset, pos.y + style.FramePadding.y));

		// Icon
		ImGui::Image(img, icon_size);

		// Label
		ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
		ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, pos.y + style.FramePadding.y));
		ImGui::TextUnformatted(label.c_str());

		// Click toggling
		if (ImGui::IsItemClicked())
		{
			is_open = !is_open;
			ImGui::TreeNodeSetOpen(node_id, is_open);
		}

		return is_open;
	}


}
