#include "Core/Log.h"

namespace aby {
	ELogColor LogMsg::color() const { 
        return static_cast<ELogColor>(level);
    }

    void Logger::set_streams(std::ostream& log_stream, std::ostream& err_stream) {
        m_LogStream = &log_stream;
        m_ErrStream = &err_stream;
    }

    std::size_t Logger::add_callback(Callback&& callback) {
        std::lock_guard lock(m_StreamMutex);
        std::size_t idx = m_Callbacks.size();
        m_Callbacks.push_back(callback);
        return idx;
    }
    void Logger::remove_callback(std::size_t idx) {
        m_Callbacks.erase(m_Callbacks.begin() + idx);
    }
    void Logger::flush() {
        bool unlock = m_StreamMutex.try_lock();
        for (std::size_t i = 0; i < m_MsgCount; i++) {
            auto& msg = m_MsgBuff[i];
            switch (msg.level) {
                case ELogLevel::LOG:
                case ELogLevel::DEBUG:
                {
                    *m_LogStream << "\033[" << static_cast<int>(msg.color()) << "m" << msg.text << "\033[0m" << '\n';
                    break;
                }
                case ELogLevel::WARN:
                case ELogLevel::ERR:
                {
                    *m_ErrStream << "\033[" << static_cast<int>(msg.color()) << "m" << msg.text << "\033[0m" << '\n';
                    break;
                }
            }
            for (auto& cb : m_Callbacks) {
                cb(msg);
            }
        }
        m_MsgCount = 0;
        if (unlock) {
            m_StreamMutex.unlock();
        }
    }
}