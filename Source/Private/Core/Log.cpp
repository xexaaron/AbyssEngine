#include "Core/Log.h"

namespace aby {
	ELogColor LogMsg::color() const { 
        return static_cast<ELogColor>(level);
    }

    void Logger::set_streams(std::ostream& log_stream, std::ostream& err_stream) {
        m_LogStream = &log_stream;
        m_ErrStream = &err_stream;
    }

    void Logger::set_only_do_cb(bool only_do_cb) {
        bOnlyDoCallbacks = only_do_cb;
    }

    std::size_t Logger::add_callback(Callback&& callback) {
        const std::size_t idx = m_Callbacks.size();
        m_Callbacks.push_back(callback);
        return idx;
    }
    void Logger::remove_callback(std::size_t idx) {
        m_Callbacks.erase(m_Callbacks.begin() + idx);
    }
    void Logger::flush() {
        static bool is_flushing = false;
        std::lock_guard lock(m_Mutex); 

        if (is_flushing) {
            return;
        }

        is_flushing = true;

        while (!m_MsgBuff.empty()) {
            auto& msg = m_MsgBuff.front();
            if (!bOnlyDoCallbacks) {
                switch (msg.level) {
                case ELogLevel::LOG:
                case ELogLevel::DEBUG:
                    *m_LogStream << "\033[" << static_cast<int>(msg.color()) << "m" << msg.text << "\033[0m" << '\n';
                    break;
                case ELogLevel::WARN:
                case ELogLevel::ERR:
                    *m_ErrStream << "\033[" << static_cast<int>(msg.color()) << "m" << msg.text << "\033[0m" << '\n';
                    break;
                }
            }
            for (auto& cb : m_Callbacks) {
                cb(msg);
            }
            m_MsgBuff.pop();
        }
        
        is_flushing = false;
    }

    std::string Logger::time_date_now() {
        auto now = std::chrono::system_clock::now();
        return std::format("{0:%F} {0:%T}", std::chrono::floor<std::chrono::seconds>(now));
    }

    std::string Logger::time_date_now_header() {
        auto now = std::chrono::system_clock::now();
        return std::format("[{0:%F}][{0:%T}]", std::chrono::floor<std::chrono::seconds>(now));
    }

    glm::vec4 Logger::log_color_to_vec4(ELogColor color) {
        switch (color) {
            using enum ELogColor;
        case Yellow: return { 1.0f, 1.0f, 0.0f, 1.0f };  // RGB(255, 255, 0)
        case Cyan:   return { 0.0f, 1.0f, 1.0f, 1.0f };  // RGB(0, 255, 255)
        case Grey:   return { 0.7f, 0.7f, 0.7f, 1.0f };  // RGB(128, 128, 128)
        case Red:    return { 1.0f, 0.0f, 0.0f, 1.0f };  // RGB(255, 0, 0)
        default:     return { 1.0f, 1.0f, 1.0f, 1.0f };  // Default to white
        }
    }


}