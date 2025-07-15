#pragma once

#include "Platform/Process.h"
#include <imgui.h>
#include <vector>

namespace aby::imgui {

    class Console  {
    public:
        Console(const std::string& title = "Console", bool is_child_window = true);
        ~Console();


        void add_msg(const char* fmt, ...);
        void add_msg(const LogMsg& msg);
        void clear();
        void draw(bool* p_open);
        void exec_cmd(const char* command_line);
    private:
        void draw_options(bool* p_open);
        void draw_filter();
        void draw_log();
        void draw_cmdline();
        int on_text_edit(ImGuiInputTextCallbackData* data);
    private:
        static void strtrim(char* s);
    private:
        std::string           m_Title;
        char                  m_InputBuf[256];
        Unique<sys::Process>  m_OpenProc;
        std::vector<LogMsg>   m_Items;
        ImVector<const char*> m_Commands;
        ImVector<char*>       m_History;
        int                   m_HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
        ImGuiTextFilter       m_Filter;
        bool                  bAutoScroll;
        bool                  bScrollToBottom;
        bool                  bCopyToClipboard;
        bool                  bChildWindow;
    };

}