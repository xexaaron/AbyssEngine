#include "Platform/imgui/imconsole.h"
#include "Platform/imgui/imwidget.h"
#include "Utility/TagParser.h"

namespace aby::imgui {
   

    Console::Console(const std::string& title, bool is_child_window) : 
        m_Title(title),
        m_OpenProc(nullptr),
        m_HistoryPos(-1),
        bAutoScroll(true),
        bScrollToBottom(false),
        bCopyToClipboard(false),
        bChildWindow(is_child_window)
    { 
        clear();
        memset(m_InputBuf, 0, sizeof(m_InputBuf));
        m_Commands.push_back("aby.help");
        m_Commands.push_back("aby.history");
        m_Commands.push_back("aby.clear");
    }

    Console::~Console() {
        clear();
        for (int i = 0; i < m_History.Size; i++)
            ImGui::MemFree(m_History[i]);
    }

    void Console::add_msg(const char* fmt, ...) {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf) - 1] = 0;
        va_end(args);
        m_Items.push_back(
            LogMsg{
                .level = ELogLevel::LOG,
                .text  = std::string(buf),
            }
        );
    }

    void Console::add_msg(const LogMsg& msg) {
        m_Items.push_back(msg);
    }

    void Console::draw(bool* p_open) {
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
        if (bChildWindow) {
            if (!ImGui::BeginChild(m_Title.c_str())) {
                ImGui::EndChild();
                return;
            }
        }
        else {
            if (!ImGui::Begin(m_Title.c_str(), p_open))
            {
                ImGui::End();
                return;
            }
        }
        

        ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 0);
        ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 0); 
        draw_options(p_open);
        ImGui::SameLine();
        draw_filter();
        ImGui::PopStyleVar(2);

        ImGui::Separator();
        draw_log();
        draw_cmdline();

        if (bChildWindow) {
            ImGui::EndChild();
        }
        else {
            ImGui::End();
        }
    }

    void Console::draw_options(bool* p_open) {
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Close Console"))
                *p_open = false;
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &bAutoScroll);
            ImGui::EndPopup();
        }
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
    }

    void Console::draw_filter() {
        ImVec4* colors = ImGui::GetStyle().Colors;
        ImGui::SetNextItemWidth(200.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, colors[ImGuiCol_WindowBg]);
        if (ImGui::InputTextWithHint(
            "##ConsoleFilter",
            "Filter (\"incl,-excl\")",
            m_Filter.InputBuf,
            IM_ARRAYSIZE(m_Filter.InputBuf))
            )
        { 
            m_Filter.Build();
        }
        ImGui::PopStyleColor();
        imgui::UnderlinePreviousText();
    }

    void Console::draw_log() {

        // Reserve enough left-over height for 1 separator + 1 input text
       
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -20), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (ImGui::BeginPopupContextWindow())
            {
                bCopyToClipboard = ImGui::Selectable("Copy All");
                if (ImGui::Selectable("Clear")) clear();

                ImGui::EndPopup();
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

            if (bCopyToClipboard)
                ImGui::LogToClipboard();

            for (const LogMsg& msg : m_Items)
            {
                const char* item = msg.text.c_str();
                if (!m_Filter.PassFilter(item))
                    continue;

                ImGui::PushStyleColor(ImGuiCol_Text, Logger::log_color_to_vec4(msg.color()));
                if (util::contains_tags(item)) {
                    imgui::TextWithTags(item, true);
                }
                else {
                    ImGui::TextWrapped(item);
                }
                ImGui::PopStyleColor();
            }

            if (bCopyToClipboard)
                ImGui::LogFinish();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (bScrollToBottom || (bAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);
            bScrollToBottom = false;

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
    }

    void Console::draw_cmdline() {
        // Command-line
        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
        ImVec4* colors = ImGui::GetStyle().Colors;
        
        ImGui::PushStyleColor(ImGuiCol_FrameBg, colors[ImGuiCol_WindowBg]);
        ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 0);
        ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 0);
        ImGui::PushStyleVarY(ImGuiStyleVar_WindowPadding, 0);

        auto te_cb = [](ImGuiInputTextCallbackData* data) -> int {
            Console* console = static_cast<Console*>(data->UserData);
            return console->on_text_edit(data);
        };
        if (ImGui::InputTextWithHint("##ConsoleInput", ">>>", m_InputBuf, IM_ARRAYSIZE(m_InputBuf), input_text_flags, te_cb, (void*)this))
        {
            char* s = m_InputBuf;
            strtrim(s);
            if (s[0]) {
                exec_cmd(s);
            }
            strcpy(s, "");
            reclaim_focus = true;
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(3);

        if (ImGui::IsItemFocused() &&
            ImGui::IsKeyDown(ImGuiMod_Ctrl) &&
            ImGui::IsKeyPressed(ImGuiKey_C, false) &&
            m_OpenProc && m_OpenProc->is_open())
        {
            m_OpenProc->kill();
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

    }

    void Console::exec_cmd(const char* command_line) {
        ABY_LOG("# {}", command_line);

        // Insert into history. First find match and delete it so it can be pushed to the back.
        // This isn't trying to be smart or optimal.
        m_HistoryPos = -1;
        for (int i = m_History.Size - 1; i >= 0; i--)
            if (stricmp(m_History[i], command_line) == 0)
            {
                ImGui::MemFree(m_History[i]);
                m_History.erase(m_History.begin() + i);
                break;
            }
        m_History.push_back(strdup(command_line));

        // Process command

        std::string cmd(command_line);

        if (cmd.starts_with("aby.")) {
            if (stricmp(command_line, "aby.clear") == 0) {
                clear();
            }
            else if (stricmp(command_line, "aby.help") == 0) {
                ABY_LOG("Commands:");
                for (int i = 0; i < m_Commands.Size; i++)
                    ABY_LOG("\t{}", m_Commands[i]);
            }
            else if (stricmp(command_line, "aby.history") == 0) {
                int first = m_History.Size - 10;
                for (int i = first > 0 ? first : 0; i < m_History.Size; i++)
                    ABY_LOG("{:3}: {}", i, m_History[i]);
            }
        } else if (cmd.starts_with("sys.")) {
            auto sys_cmd = cmd.substr(4);
            
            if (m_OpenProc && m_OpenProc->is_open()) {
                m_OpenProc->write(sys_cmd);
            }
            else {
                m_OpenProc = sys::Process::create([](const std::string& out) {
                    ABY_LOG("{}", out);
                });
                m_OpenProc->open(sys_cmd);
            }
         
        } else {
            ABY_ERR("Unknown command {}", command_line);
            ABY_WARN("Commands are context prefixed");
            ABY_WARN("\tFor AbyssEngine commands use 'aby.CMD'");
            ABY_WARN("\tFor System commands use 'sys.CMD'");
            ABY_WARN("\tFor Python commands use 'py.CMD'");
        }



        // On command input, we scroll to bottom even if AutoScroll==false
        bScrollToBottom = true;
    }

    int Console::on_text_edit(ImGuiInputTextCallbackData* data) {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackCompletion:
        {
            // Example of TEXT COMPLETION

            // Locate beginning of current word
            const char* word_end = data->Buf + data->CursorPos;
            const char* word_start = word_end;
            while (word_start > data->Buf)
            {
                const char c = word_start[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                    break;
                word_start--;
            }

            // Build a list of candidates
            ImVector<const char*> candidates;
            for (int i = 0; i < m_Commands.Size; i++)
                if (strnicmp(m_Commands[i], word_start, (int)(word_end - word_start)) == 0)
                    candidates.push_back(m_Commands[i]);

            if (candidates.Size == 0)
            {
                // No match
                ABY_ERR("No match for \"{}\"!", std::string(word_start, word_end));
            }
            else if (candidates.Size == 1)
            {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            }
            else
            {
                // Multiple matches. Complete as much as we can..
                // So inputting "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                int match_len = (int)(word_end - word_start);
                for (;;)
                {
                    int c = 0;
                    bool all_candidates_matches = true;
                    for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                        if (i == 0)
                            c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                    if (!all_candidates_matches)
                        break;
                    match_len++;
                }

                if (match_len > 0)
                {
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                }

                // List matches
                ABY_ERR("Possible matches:");
                for (int i = 0; i < candidates.Size; i++)
                    ABY_ERR("\t{}", candidates[i]);
            }

            break;
        }
        case ImGuiInputTextFlags_CallbackHistory:
        {
            // Example of HISTORY
            const int prev_history_pos = m_HistoryPos;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (m_HistoryPos == -1)
                    m_HistoryPos = m_History.Size - 1;
                else if (m_HistoryPos > 0)
                    m_HistoryPos--;
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (m_HistoryPos != -1)
                    if (++m_HistoryPos >= m_History.Size)
                        m_HistoryPos = -1;
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (prev_history_pos != m_HistoryPos)
            {
                const char* history_str = (m_HistoryPos >= 0) ? m_History[m_HistoryPos] : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
        }
        return 0;
    }

    void Console::clear() {
        m_Items.clear();
    }

    void Console::strtrim(char* s) {
        char* str_end = s + strlen(s);
        while (str_end > s && str_end[-1] == ' ')
            str_end--; *str_end = 0;
    }

}