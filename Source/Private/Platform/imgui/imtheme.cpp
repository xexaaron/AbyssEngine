#include "Platform/imgui/imtheme.h"
#include "Utility/Serialize.h"

namespace aby::imgui {
    
    Theme::Theme(const std::string& name) : m_Name(name) {
        ABY_ASSERT(name.size() <= 128, "Name cannot be longer than 128 characters");
    }

    Theme::Theme(const std::string& name, const std::filesystem::path& dir) :
        m_Name(name)
    {
        ABY_ASSERT(name.size() <= 128, "Name cannot be longer than 128 characters");
        util::SerializeOpts opts{ dir / (m_Name + ".imtheme"), util::ESerializeMode::READ};
        util::Serializer s(opts);
        s.read(m_Style.Alpha);
        s.read(m_Style.DisabledAlpha);
        s.read(m_Style.WindowPadding.x);
        s.read(m_Style.WindowPadding.y);
        s.read(m_Style.WindowRounding);
        s.read(m_Style.WindowBorderSize);
        s.read(m_Style.WindowBorderHoverPadding);
        s.read(m_Style.WindowMinSize.x);
        s.read(m_Style.WindowMinSize.y);
        s.read(m_Style.WindowTitleAlign.x);
        s.read(m_Style.WindowTitleAlign.y);
        s.read(m_Style.WindowMenuButtonPosition);
        s.read(m_Style.ChildRounding);
        s.read(m_Style.ChildBorderSize);
        s.read(m_Style.PopupRounding);
        s.read(m_Style.PopupBorderSize);
        s.read(m_Style.FramePadding.x);
        s.read(m_Style.FramePadding.y);
        s.read(m_Style.FrameRounding);
        s.read(m_Style.FrameBorderSize);
        s.read(m_Style.ItemSpacing.x);
        s.read(m_Style.ItemSpacing.y);
        s.read(m_Style.ItemInnerSpacing.x);
        s.read(m_Style.ItemInnerSpacing.y);
        s.read(m_Style.CellPadding.x);
        s.read(m_Style.CellPadding.y);
        s.read(m_Style.TouchExtraPadding.x);
        s.read(m_Style.TouchExtraPadding.y);
        s.read(m_Style.IndentSpacing);
        s.read(m_Style.ColumnsMinSpacing);
        s.read(m_Style.ScrollbarSize);
        s.read(m_Style.ScrollbarRounding);
        s.read(m_Style.GrabMinSize);
        s.read(m_Style.GrabRounding);
        s.read(m_Style.LogSliderDeadzone);
        s.read(m_Style.ImageBorderSize);
        s.read(m_Style.TabRounding);
        s.read(m_Style.TabBorderSize);
        s.read(m_Style.TabCloseButtonMinWidthSelected);
        s.read(m_Style.TabCloseButtonMinWidthUnselected);
        s.read(m_Style.TabBarBorderSize);
        s.read(m_Style.TabBarOverlineSize);
        s.read(m_Style.TableAngledHeadersAngle);
        s.read(m_Style.TableAngledHeadersTextAlign.x);
        s.read(m_Style.TableAngledHeadersTextAlign.y);
        s.read(m_Style.TreeLinesFlags);
        s.read(m_Style.TreeLinesSize);
        s.read(m_Style.TreeLinesRounding);
        s.read(m_Style.ColorButtonPosition);
        s.read(m_Style.ButtonTextAlign.x);
        s.read(m_Style.ButtonTextAlign.y);
        s.read(m_Style.SelectableTextAlign.x);
        s.read(m_Style.SelectableTextAlign.y);
        s.read(m_Style.SeparatorTextBorderSize);
        s.read(m_Style.SeparatorTextAlign.x);
        s.read(m_Style.SeparatorTextAlign.y);
        s.read(m_Style.SeparatorTextPadding.x);
        s.read(m_Style.SeparatorTextPadding.y);
        s.read(m_Style.DisplayWindowPadding.x);
        s.read(m_Style.DisplayWindowPadding.y);
        s.read(m_Style.DisplaySafeAreaPadding.x);
        s.read(m_Style.DisplaySafeAreaPadding.y);
        s.read(m_Style.DockingSeparatorSize);
        s.read(m_Style.MouseCursorScale);
        s.read(m_Style.AntiAliasedLines);
        s.read(m_Style.AntiAliasedLinesUseTex);
        s.read(m_Style.AntiAliasedFill);
        s.read(m_Style.CurveTessellationTol);
        s.read(m_Style.CircleTessellationMaxError);
        for (std::size_t i = 0; i < ImGuiCol_COUNT; i++) {
            s.read(m_Style.Colors[i].x);
            s.read(m_Style.Colors[i].y);
            s.read(m_Style.Colors[i].z);
            s.read(m_Style.Colors[i].w);
        }
        s.read(m_Style.HoverStationaryDelay);
        s.read(m_Style.HoverDelayShort);
        s.read(m_Style.HoverDelayNormal);
        s.read(m_Style.HoverFlagsForTooltipMouse);
        s.read(m_Style.HoverFlagsForTooltipNav);
    }

    Theme::Theme(const std::string& name, ImGuiStyle* style) : 
        m_Name(name),
        m_Style(*style)
    {
        ABY_ASSERT(name.size() <= 128, "Name cannot be longer than 128 characters");
    }

    void Theme::save(const std::filesystem::path& dir) {
        if (!std::filesystem::exists(dir)) {
            std::filesystem::create_directory(dir);
        }

        util::SerializeOpts opts{ dir / (m_Name + ".imtheme"), util::ESerializeMode::WRITE };
        util::Serializer s(opts);
        s.write(m_Style.Alpha);
        s.write(m_Style.DisabledAlpha);
        s.write(m_Style.WindowPadding.x);
        s.write(m_Style.WindowPadding.y);
        s.write(m_Style.WindowRounding);
        s.write(m_Style.WindowBorderSize);
        s.write(m_Style.WindowBorderHoverPadding);
        s.write(m_Style.WindowMinSize.x);
        s.write(m_Style.WindowMinSize.y);
        s.write(m_Style.WindowTitleAlign.x);
        s.write(m_Style.WindowTitleAlign.y);
        s.write(m_Style.WindowMenuButtonPosition);
        s.write(m_Style.ChildRounding);
        s.write(m_Style.ChildBorderSize);
        s.write(m_Style.PopupRounding);
        s.write(m_Style.PopupBorderSize);
        s.write(m_Style.FramePadding.x);
        s.write(m_Style.FramePadding.y);
        s.write(m_Style.FrameRounding);
        s.write(m_Style.FrameBorderSize);
        s.write(m_Style.ItemSpacing.x);
        s.write(m_Style.ItemSpacing.y);
        s.write(m_Style.ItemInnerSpacing.x);
        s.write(m_Style.ItemInnerSpacing.y);
        s.write(m_Style.CellPadding.x);
        s.write(m_Style.CellPadding.y);
        s.write(m_Style.TouchExtraPadding.x);
        s.write(m_Style.TouchExtraPadding.y);
        s.write(m_Style.IndentSpacing);
        s.write(m_Style.ColumnsMinSpacing);
        s.write(m_Style.ScrollbarSize);
        s.write(m_Style.ScrollbarRounding);
        s.write(m_Style.GrabMinSize);
        s.write(m_Style.GrabRounding);
        s.write(m_Style.LogSliderDeadzone);
        s.write(m_Style.ImageBorderSize);
        s.write(m_Style.TabRounding);
        s.write(m_Style.TabBorderSize);
        s.write(m_Style.TabCloseButtonMinWidthSelected);
        s.write(m_Style.TabCloseButtonMinWidthUnselected);
        s.write(m_Style.TabBarBorderSize);
        s.write(m_Style.TabBarOverlineSize);
        s.write(m_Style.TableAngledHeadersAngle);
        s.write(m_Style.TableAngledHeadersTextAlign.x);
        s.write(m_Style.TableAngledHeadersTextAlign.y);
        s.write(m_Style.TreeLinesFlags);
        s.write(m_Style.TreeLinesSize);
        s.write(m_Style.TreeLinesRounding);
        s.write(m_Style.ColorButtonPosition);
        s.write(m_Style.ButtonTextAlign.x);
        s.write(m_Style.ButtonTextAlign.y);
        s.write(m_Style.SelectableTextAlign.x);
        s.write(m_Style.SelectableTextAlign.y);
        s.write(m_Style.SeparatorTextBorderSize);
        s.write(m_Style.SeparatorTextAlign.x);
        s.write(m_Style.SeparatorTextAlign.y);
        s.write(m_Style.SeparatorTextPadding.x);
        s.write(m_Style.SeparatorTextPadding.y);
        s.write(m_Style.DisplayWindowPadding.x);
        s.write(m_Style.DisplayWindowPadding.y);
        s.write(m_Style.DisplaySafeAreaPadding.x);
        s.write(m_Style.DisplaySafeAreaPadding.y);
        s.write(m_Style.DockingSeparatorSize);
        s.write(m_Style.MouseCursorScale);
        s.write(m_Style.AntiAliasedLines);
        s.write(m_Style.AntiAliasedLinesUseTex);
        s.write(m_Style.AntiAliasedFill);
        s.write(m_Style.CurveTessellationTol);
        s.write(m_Style.CircleTessellationMaxError);
        
        for (std::size_t i = 0; i < ImGuiCol_COUNT; i++) {
            s.write(m_Style.Colors[i].x);
            s.write(m_Style.Colors[i].y);
            s.write(m_Style.Colors[i].z);
            s.write(m_Style.Colors[i].w);
        }
        s.write(m_Style.HoverStationaryDelay);
        s.write(m_Style.HoverDelayShort);
        s.write(m_Style.HoverDelayNormal);
        s.write(m_Style.HoverFlagsForTooltipMouse);
        s.write(m_Style.HoverFlagsForTooltipNav);
        s.save();
    }

    bool Theme::exists(const std::string& name, const std::filesystem::path& dir) {
        return std::filesystem::exists(dir / (name + ".imtheme"));
    }


    void Theme::set_current() {
        ImGui::GetStyle() = m_Style;
    }

    ImGuiStyle* Theme::style() {
        return &m_Style;
    }
    std::string& Theme::name() {
        return m_Name;
    }

}